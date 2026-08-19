#ifndef DATASTAX_EXPORTED_CONNECTION_HPP
#define DATASTAX_EXPORTED_CONNECTION_HPP

#include "ref_counted.hpp"
#include "protocol.hpp"
#include "address.hpp"

namespace datastax { namespace internal { namespace core {

class Host;
class Connection;
class ConnectionListener;
class SocketHandlerBase;

/**
 * This class is a hack that allows moving Connection to
 * different event loop. libuv doesn't have any mechanism
 * to do that (https://github.com/libuv/libuv/issues/390).
 * The solution used here is to extract file descriptor from Connection,
 * along with some important fields of Connection and Socket,
 * duplicate fd it using `dup` syscall, close and destroy original Connection,
 * and then on destination event loop create new Coonnection and Socket objects,
 * restoring their state from saved fields.
 */
class ExportedConnection : public RefCounted<ExportedConnection> {
public:
    typedef SharedRefPtr<ExportedConnection> Ptr;
    ExportedConnection(SharedRefPtr<Connection> connection);
    ~ExportedConnection();
    SharedRefPtr<Connection> import_connection(uv_loop_t *loop);

private:
    // Connection fields
    SharedRefPtr<Host> host;
    ConnectionListener* listener_;
    ProtocolVersion protocol_version_;
    String keyspace_;
    int32_t shard_id_ = 0;
    unsigned int idle_timeout_secs_;
    unsigned int heartbeat_interval_secs_;

    // Socket fields
    int fd;
    SocketHandlerBase *handler_;
    bool is_defunct_;
    size_t max_reusable_write_objects_;
    Address address_;
};

}}}  // namespace datastax::internal::core

#endif
