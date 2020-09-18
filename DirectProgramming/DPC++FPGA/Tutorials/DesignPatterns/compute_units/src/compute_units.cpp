//==============================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================
#include <CL/sycl.hpp>
#include <iostream>

// dpc_common.hpp can be found in the dev-utilities include folder,
// e.g., $ONEAPI_ROOT/dev-utilities/latest/include/dpc_common.hpp
#include "dpc_common.hpp"
#include "compute_units.hpp"
#include "pipe_array.hpp"

// Header locations and some DPC++ extensions changed between beta09 and beta10
// Temporarily modify the code sample to accept either version
#define BETA09 20200827
#if __SYCL_COMPILER_VERSION <= BETA09
  #include <CL/sycl/intel/fpga_extensions.hpp>
  namespace INTEL = sycl::intel;  // Namespace alias for backward compatibility
#else
  #include <CL/sycl/INTEL/fpga_extensions.hpp>
#endif


using namespace sycl;

constexpr float kTestData = 555;
constexpr size_t kEngines = 5;

using Pipes = PipeArray<class MyPipe, float, 1, kEngines + 1>;

// Forward declaration of the kernel name
// (This will become unnecessary in a future compiler version.)
class Source;
class Sink;
template <std::size_t ID> class ChainComputeUnit;

// Write the data into the chain
void SourceKernel(queue &q, float data) {

  q.submit([&](handler &h) {
    h.single_task<Source>([=] { Pipes::PipeAt<0>::write(data); });
  });
}

// Get the data out of the chain and return it to the host
void SinkKernel(queue &q, std::array<float, 1> &out_data) {

  // Use verbose SYCL 1.2 syntax for the output buffer.
  // (This will become unnecessary in a future compiler version.)
  buffer<float, 1> out_buf(out_data.data(), 1);

  q.submit([&](handler &h) {
    auto out_accessor = out_buf.get_access<access::mode::write>(h);
    h.single_task<Sink>(
        [=] { out_accessor[0] = Pipes::PipeAt<kEngines>::read(); });
  });
}

int main() {

#if defined(FPGA_EMULATOR)
  INTEL::fpga_emulator_selector device_selector;
#else
  INTEL::fpga_selector device_selector;
#endif

  std::array<float, 1> out_data = {0};

  try {
    queue q(device_selector, dpc_common::exception_handler);

    // Enqueue the Source kernel
    SourceKernel(q, kTestData);

    // Enqueue the chain of kEngines compute units
    // Compute unit must take a single argument, its ID
    submit_compute_units<kEngines, ChainComputeUnit>(q, [=](auto ID) {
      float f = Pipes::PipeAt<ID>::read();
      Pipes::PipeAt<ID + 1>::write(f);
    });

    // Enqueue the Sink kernel
    SinkKernel(q, out_data);

  } catch (sycl::exception const &e) {
    // Catches exceptions in the host code
    std::cout << "Caught a SYCL host exception:\n" << e.what() << "\n";

    // Most likely the runtime couldn't find FPGA hardware!
    if (e.get_cl_code() == CL_DEVICE_NOT_FOUND) {
      std::cout << "If you are targeting an FPGA, please ensure that your "
                   "system has a correctly configured FPGA board.\n";
      std::cout << "If you are targeting the FPGA emulator, compile with "
                   "-DFPGA_EMULATOR.\n";
    }
    std::terminate();
  }

  // Verify result
  if (out_data[0] != kTestData) {
    std::cout << "FAILED: The results are incorrect\n";
    std::cout << "Expected: " << kTestData << " Got: " << out_data[0] << "\n";
    return 1;
  }

  std::cout << "PASSED: The results are correct\n";
  return 0;
}
