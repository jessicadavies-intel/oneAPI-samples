#ifndef __GZIPKERNEL_H__
#define __GZIPKERNEL_H__
#pragma once

#include <CL/sycl.hpp>

extern "C" void SubmitGzipTasks(
    sycl::queue &sycl_device,
    size_t block_size,  // size of block to compress.
    sycl::buffer<char, 1> *pibuf, sycl::buffer<char, 1> *pobuf,
    sycl::buffer<struct GzipOutInfo, 1> *gzip_out_buf,
    sycl::buffer<unsigned, 1> *current_crc, bool last_block, sycl::event &e_crc,
    sycl::event &e_lz, sycl::event &e_huff, size_t engineID);

#endif  //__GZIPKERNEL_H__
