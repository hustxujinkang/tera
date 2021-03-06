// Copyright (c) 2015-2017, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "observer/rowlocknode/zk_rowlock_client_zk_adapter.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "sdk/rowlock_client.h"
#include "types.h"

DECLARE_string(rowlock_zk_root_path);
DECLARE_string(tera_zk_addr_list);
DECLARE_int32(rowlock_server_node_num);
DECLARE_int64(tera_zk_retry_period); 
DECLARE_int32(tera_zk_timeout);
DECLARE_int32(tera_zk_retry_max_times);

namespace tera {
namespace observer {

ZkRowlockClientZkAdapter::ZkRowlockClientZkAdapter(RowlockClient* server_client, 
											   const std::string& server_addr)
    : client_(server_client),
      server_addr_(server_addr) {}

ZkRowlockClientZkAdapter::~ZkRowlockClientZkAdapter() {
    ZooKeeperAdapter::Finalize();
}
      
bool ZkRowlockClientZkAdapter::Init() {
    std::string root_path = FLAGS_rowlock_zk_root_path;
    std::string proxy_path = root_path + kRowlockProxyPath;

    int zk_errno = zk::ZE_OK;;
    // init zk client
    while (!ZooKeeperAdapter::Init(FLAGS_tera_zk_addr_list,
                                   FLAGS_rowlock_zk_root_path, FLAGS_tera_zk_timeout,
                                   server_addr_, &zk_errno)) {
        LOG(ERROR) << "fail to init zk : " << zk::ZkErrnoToString(zk_errno);
        ThisThread::Sleep(FLAGS_tera_zk_retry_period);
    }
    LOG(INFO) << "init zk success";

    std::vector<std::string> child;
    std::vector<std::string> value;

    while (!ListChildren(proxy_path, &child, &value, &zk_errno)) {
    	LOG(ERROR) << "fail to get proxy addr : " << zk::ZkErrnoToString(zk_errno);
        ThisThread::Sleep(FLAGS_tera_zk_retry_period);
    }
    client_->Update(value);
    return true;
}

} // namespace observer
} // namespace tera