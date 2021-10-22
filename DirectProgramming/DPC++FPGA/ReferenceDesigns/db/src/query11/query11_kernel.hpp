#ifndef __QUERY11_KERNEL_HPP__
#define __QUERY11_KERNEL_HPP__
#pragma once

#include <CL/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

#include "../dbdata.hpp"

bool SubmitQuery11(sycl::queue& q, Database& dbinfo,
                   std::string& nation,
                   std::vector<DBIdentifier>& partkeys,
                   std::vector<DBDecimal>& values,
                   double& kernel_latency, double& total_latency);

#endif  //__QUERY11_KERNEL_HPP__
