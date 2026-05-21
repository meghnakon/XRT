/**
 * Copyright (C) 2020 Xilinx, Inc
 * Copyright (C) 2024 Advanced Micro Devices, Inc. - All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include <functional>

#include "hal_device_offload.h"
#include "core/common/module_loader.h"
#include "core/common/dlfcn.h"
#include "core/common/config_reader.h"

namespace xdp {
namespace hal {
namespace device_offload {

  void load()
  {
    static xrt_core::module_loader
      xdp_hal_device_offload_loader("xdp_hal_device_offload_plugin",
                                    register_functions,
                                    warning_function,
                                    error_function);
  }

  std::function<void (void*, bool)> update_device_cb;
  std::function<void (void*)> flush_device_cb;
  std::function<int  (void)>  get_app_style_cb;

  void register_functions(void* handle)
  {
    using ftype = void (*)(void*);
    using utype = void (*)(void*, bool);
    using gtype = int  (*)(void);

    update_device_cb = reinterpret_cast<utype>(xrt_core::dlsym(handle, "updateDeviceHAL"));

    flush_device_cb = reinterpret_cast<ftype>(xrt_core::dlsym(handle, "flushDeviceHAL"));

    get_app_style_cb = reinterpret_cast<gtype>(xrt_core::dlsym(handle, "getAppStyleHAL"));
  }

  void warning_function()
  {
    // No warnings at this level
  }

  int error_function()
  {
    return 0;
  }

} // end namespace device_offload

  void flush_device(void* handle)
  {
    if (device_offload::flush_device_cb != nullptr)
      device_offload::flush_device_cb(handle);
  }

  void update_device(void* handle, bool hw_context_flow)
  {
    if (device_offload::update_device_cb != nullptr)
      device_offload::update_device_cb(handle, hw_context_flow);
  }

  int get_app_style()
  {
    if (device_offload::get_app_style_cb != nullptr)
      return device_offload::get_app_style_cb();
    // Plugin not loaded (e.g. tracing disabled). Report NOT_SET so callers
    // treat this as "no XDP state to honour" and proceed with default behavior.
    return 0;
  }
} // end namespace hal
} // end namespace xdp
