#ifndef GRPC_OPENTRACING_PROPAGATION_H
#define GRPC_OPENTRACING_PROPAGATION_H

#include <grpc++/server_context.h>
#include <opentracing/propagation.h>

namespace grpc_opentracing {

class Reader : public opentracing::HTTPHeadersReader {
  public:
    explicit Reader(const grpc::ServerContext& context)
        : _context(context)
    {
    }

    opentracing::expected<opentracing::string_view>
    LookupKey(opentracing::string_view key) const override
    {
        const auto& metadata = _context.client_metadata();
        auto itr = metadata.find(grpc::string_ref(key.data(), key.length()));
        if (itr == std::end(metadata)) {
            return opentracing::make_unexpected(
                opentracing::key_not_found_error);
        }
        return opentracing::expected<opentracing::string_view>(
            opentracing::string_view(itr->second.data(), itr->second.length()));
    }

    opentracing::expected<void> ForeachKey(
        std::function<opentracing::expected<void>(
            opentracing::string_view key, opentracing::string_view value)> f)
        const override
    {
        const auto& metadata = _context.client_metadata();
        for (auto&& pair : metadata) {
            const opentracing::string_view key(
                pair.first.data(), pair.first.length());
            const opentracing::string_view value(
                pair.second.data(), pair.second.length());
            auto result = f(key, value);
            if (!result) {
                return result;
            }
        }
        return opentracing::expected<void>();
    }

  private:
    const grpc::ServerContext& _context;
};

class Writer : public opentracing::HTTPHeadersWriter {
  public:
    explicit Writer(grpc::ServerContext& context)
        : _context(context)
    {
    }

    opentracing::expected<void> Set(
        opentracing::string_view key, opentracing::string_view value)
        const override
    {
        _context.AddInitialMetadata(key, value);
        return opentracing::expected<void>();
    }

  private:
    grpc::ServerContext& _context;
};

}  // namespace grpc_opentracing

#endif  // GRPC_OPENTRACING_PROPAGATION_H
