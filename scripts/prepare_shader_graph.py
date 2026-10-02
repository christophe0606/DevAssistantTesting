"""Expose shader operators to PT2E before the Arm decomposition passes.

The Arm quantizer annotates clamp/where.self/slice, but the initial PyTorch
export contains clamp_min/max, scalar where overloads and tuple split outputs.
Normalize those forms before calibration so the complete tensor stage can
remain inside an integer Ethos delegate.
"""
import operator
import torch


def prepare_shader_graph(module):
    graph=module.graph
    aten=torch.ops.aten
    for node in list(graph.nodes):
        if node.target==aten.clamp_min.default:
            node.target=aten.clamp.default
            node.args=(node.args[0],node.args[1],None)
        elif node.target==aten.clamp_max.default:
            node.target=aten.clamp.default
            node.args=(node.args[0],None,node.args[1])
        elif node.target in (aten.where.ScalarOther,aten.where.ScalarSelf,aten.where.Scalar):
            condition,x,y=node.args
            reference=x if isinstance(x,torch.fx.Node) else y if isinstance(y,torch.fx.Node) else condition
            def tensor(value):
                if isinstance(value,torch.fx.Node):
                    return value
                with graph.inserting_before(node):
                    const=graph.call_function(aten.full_like.default,(reference,value),{'dtype':node.meta['val'].dtype})
                const.meta=node.meta.copy()
                const.meta['val']=torch.empty_like(reference.meta['val'],dtype=node.meta['val'].dtype)
                return const
            node.target=aten.where.self
            node.args=(condition,tensor(x),tensor(y))
        elif node.target in (aten.split.Tensor,aten.split_with_sizes.default):
            source,widths,*dim_arg=node.args
            dim=dim_arg[0] if dim_arg else 0
            size=source.meta['val'].shape[dim]
            if isinstance(widths,int):
                widths=[min(widths,size-start) for start in range(0,size,widths)]
            offsets=[0]
            for width in widths:
                offsets.append(offsets[-1]+width)
            for user in list(node.users):
                if user.target!=operator.getitem:
                    raise ValueError('Shader split must have static getitem users')
                index=user.args[1]
                with graph.inserting_before(user):
                    sliced=graph.call_function(aten.slice.Tensor,(source,dim,offsets[index],offsets[index+1]))
                sliced.meta=user.meta.copy()
                user.replace_all_uses_with(sliced)
                graph.erase_node(user)
            graph.erase_node(node)
    graph.lint()
    module.recompile()
    return module
