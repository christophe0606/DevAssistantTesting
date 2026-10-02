/* Project-owned small bare-metal runner using the pack's Program/Method API. */
#include "model_runtime.h"
#include "model_pte.h"
#include "model_io.h"
#include <array>
#include <cstring>
#include <optional>
#include <executorch/extension/data_loader/buffer_data_loader.h>
#include <executorch/runtime/core/exec_aten/exec_aten.h>
#include <executorch/runtime/core/hierarchical_allocator.h>
#include <executorch/runtime/core/memory_allocator.h>
#include <executorch/runtime/executor/memory_manager.h>
#include <executorch/runtime/executor/program.h>
#include <executorch/runtime/executor/method.h>
#include <executorch/runtime/platform/runtime.h>

using namespace executorch::runtime;
using namespace executorch::aten;
using executorch::extension::BufferDataLoader;

namespace {
#ifndef SDF_PLANNED_ARENA_BYTES
#define SDF_PLANNED_ARENA_BYTES (2*1024*1024)
#endif
#ifndef SDF_TEMP_ARENA_BYTES
#define SDF_TEMP_ARENA_BYTES (3*1024*1024)
#endif
alignas(32) uint8_t planned[SDF_PLANNED_ARENA_BYTES] __attribute__((section(".bss.ai_pool")));
alignas(32) uint8_t scratch[SDF_TEMP_ARENA_BYTES] __attribute__((section(".bss.ai_temp_pool")));
alignas(32) uint8_t metadata[256*1024];
MemoryAllocator method_allocator(sizeof(metadata),metadata);
MemoryAllocator temp_allocator(sizeof(scratch),scratch);
std::array<Span<uint8_t>,8> spans;
std::optional<HierarchicalAllocator> hierarchy;
std::optional<MemoryManager> manager;
std::optional<BufferDataLoader> loader;
std::optional<Program> program;
std::optional<Method> current_method;
const char *current_name;

int select_method(const char *name)
{
    if (current_method && current_name && std::strcmp(current_name,name)==0) return 0;
    current_method.reset();
    manager.reset(); hierarchy.reset();
    method_allocator.reset(); temp_allocator.reset();
    auto meta=program->method_meta(name);
    if (!meta.ok()) return static_cast<int>(meta.error());
    const size_t count=meta->num_memory_planned_buffers();
    if (count>spans.size()) return -101;
    size_t offset=0;
    for (size_t i=0;i<count;++i) {
        auto result=meta->memory_planned_buffer_size(i);
        if (!result.ok()) return static_cast<int>(result.error());
        offset=(offset+31U)&~31U;
        if (*result>sizeof(planned)-offset) return -102;
        spans[i]=Span<uint8_t>(planned+offset,*result);
        offset+=*result;
    }
    hierarchy.emplace(Span<Span<uint8_t>>(spans.data(),count));
    manager.emplace(&method_allocator,&*hierarchy,&temp_allocator);
    auto result=program->load_method(name,&*manager);
    if (!result.ok()) return static_cast<int>(result.error());
    current_method.emplace(std::move(*result));
    current_name=name;
    return 0;
}
}

int model_init()
{
    runtime_init();
    if (model_pte_size==0) return -100; // Requires Create AI layer, never run a dummy model.
    loader.emplace(model_pte,model_pte_size);
    auto result=Program::load(&*loader);
    if (!result.ok()) return static_cast<int>(result.error());
    program.emplace(std::move(*result));
    return 0;
}

int model_call(const char *name,float *input,unsigned h,unsigned w,unsigned channels,
               float *output,unsigned output_elements)
{
    int error=select_method(name);
    if (error) return error;
    std::array<SizesType,4> sizes{1,static_cast<SizesType>(h),static_cast<SizesType>(w),static_cast<SizesType>(channels)};
    std::array<DimOrderType,4> order{0,1,2,3};
    TensorImpl impl(ScalarType::Float,4,sizes.data(),input,order.data());
    Tensor tensor(&impl);
    auto status=current_method->set_input(EValue(tensor),0);
    if (status!=Error::Ok) return static_cast<int>(status);
    status=current_method->execute();
    if (status!=Error::Ok) return static_cast<int>(status);
    const EValue &out=current_method->get_output(0);
    if (!out.isTensor()) return -103;
    const Tensor result=out.toTensor();
    if (result.scalar_type()!=ScalarType::Float || result.numel()!=output_elements) return -104;
    // Copies before a method switch, which reuses the planned-memory arena.
    // It also makes recurrent input/output aliasing safe.
    std::memcpy(output,result.const_data_ptr<float>(),output_elements*sizeof(float));
    return 0;
}
