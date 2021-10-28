#include <CL/sycl.hpp>
#include <utility>
// Included from DirectProgramming/DPC++FPGA/include
#include "unrolled_loop.hpp"

template <std::size_t N,                           // Number of compute units
          template <std::size_t ID> typename Name, // Name for the compute units
          typename Func>                           // Callable defining compute
                                                   // units' functionality

// Func must take a single argument. This argument is the compute unit's ID.
// The compute unit ID is a constexpr, and it can be used to specialize
// the kernel's functionality.
// Note: the type of Func's single argument must be 'auto', because Func
// will be called with various indices (i.e., the ID for each compute unit)
constexpr void SubmitComputeUnits(sycl::queue &q, Func &&f) {

  fpga_tools::UnrolledLoop<N>([&](auto i) {
    q.single_task<Name<i>>([=] {
      static_assert(
          std::is_invocable_v<Func, std::integral_constant<std::size_t, i>>,
          "The callable Func passed to SubmitComputeUnits must take a single "
          "argument of type auto");
      f(std::integral_constant<std::size_t, i>());
    });
  }); 
}
