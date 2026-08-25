#include <unistd.h>

#include "exported_connection.hpp"
#include "connection.hpp"


namespace datastax { namespace internal { namespace core {

ExportedConnection::ExportedConnection(SharedRefPtr<Connection> connection) {
    // Connection
    this->host = connection->host();
    this->listener_ = connection->listener_;
    // We don't want to notify higher layers of code
    // that connection has been closed.
    connection->set_listener();
    this->protocol_version_ = connection->protocol_version();
    this->keyspace_ = connection->keyspace();
    this->shard_id_ = connection->shard_id();
    this->idle_timeout_secs_ = connection->idle_timeout_secs_;
    this->heartbeat_interval_secs_ = connection->heartbeat_interval_secs_;

    // Socket
    uv_fileno(reinterpret_cast<uv_handle_t *>(&connection->socket_->tcp_), &this->fd);
    this->fd = dup(this->fd);
    this->handler_ = connection->socket_->handler_.release();
    // Set basic handler, to notify Connection about closing, and destroy it.
    connection->socket_->set_handler(new ConnectionHandler(connection.get()));
    this->is_defunct_ = connection->socket_->is_defunct();
    this->max_reusable_write_objects_ = connection->socket_->max_reusable_write_objects_;
    this->address_ = connection->socket_->address();

    connection->close();
}

ExportedConnection::~ExportedConnection() {
    if(handler_) {
        delete handler_;
    }
}

SharedRefPtr<Connection> ExportedConnection::import_connection(uv_loop_t *loop) {
    Socket::Ptr socket_(new Socket(this->address_, this->max_reusable_write_objects_));
    if (uv_tcp_init(loop, socket_->handle()) != 0) {
        return Connection::Ptr();
    }
    socket_->is_defunct_ = this->is_defunct_;
    uv_tcp_open(socket_->handle(), this->fd);

    Connection::Ptr connection_(new Connection(socket_, this->host, this->protocol_version_, this->idle_timeout_secs_,
                                     this->heartbeat_interval_secs_));
    connection_->set_listener(this->listener_);
    connection_->keyspace_ = keyspace_;
    connection_->set_shard_id(this->shard_id_);

    ConnectionHandler *c_handler = dynamic_cast<ConnectionHandler *>(this->handler_);
    SslConnectionHandler *s_handler = dynamic_cast<SslConnectionHandler *>(this->handler_);
    if (c_handler != nullptr) {
        c_handler->set_connection(connection_.get());
    } else if (s_handler != nullptr) {
        s_handler->set_connection(connection_.get());
    } else {
        return Connection::Ptr();
    }
    socket_->set_handler(this->handler_);
    this->handler_ = nullptr;

    socket_->inc_ref();

    return connection_;
}

}}}  // namespace datastax::internal::core