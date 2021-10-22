#ifndef __QUERY1_KERNEL_HPP__
#define __QUERY1_KERNEL_HPP__
#pragma once

#include <CL/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

#include "../dbdata.hpp"

bool SubmitQuery1(sycl::queue& q, Database& dbinfo, DBDate low_date,
                  std::array<DBDecimal, kQuery1OutSize>& sum_qty,
                  std::array<DBDecimal, kQuery1OutSize>& sum_base_price,
                  std::array<DBDecimal, kQuery1OutSize>& sum_disc_price,
                  std::array<DBDecimal, kQuery1OutSize>& sum_charge,
                  std::array<DBDecimal, kQuery1OutSize>& avg_qty,
                  std::array<DBDecimal, kQuery1OutSize>& avg_price,
                  std::array<DBDecimal, kQuery1OutSize>& avg_discount,
                  std::array<DBDecimal, kQuery1OutSize>& count,
                  double& kernel_latency, double& total_latency);

#endif  //__QUERY1_KERNEL_HPP__
