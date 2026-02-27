#pragma once

#include <expected>

#include <argparse/argparse.hpp>
#include <cpr/cpr.h>
#include <opentype-sanitiser.h>
#include <ots-memory-stream.h>

#include <incplot/desired_plot.hpp>
#include <incplot/err.hpp>

#include <uri.hpp>
#include <err.hpp>


namespace incom {
namespace terminal_plot {
namespace cl_args {
namespace incplot = incom::terminal_plot;

inline std::expected<std::vector<std::byte>, incerr_c> sanitize_fontOTS(std::span<const std::byte> bytes) {
    using enum incplot::Unexp_AP;
    if (bytes.empty()) { return std::unexpected(incerr_c::make(FONT_OTS_emptyInput)); }

    class QuietOtsContext final : public ots::OTSContext {
    public:
        void Message(int, const char *, ...) override {}
    };

    const auto inSize = bytes.size();
    size_t     limit  = inSize;
    if (inSize <= (std::numeric_limits<size_t>::max() / 4)) { limit = inSize * 4; }

    ots::ExpandingMemoryStream stream(inSize, limit);
    QuietOtsContext            context;

    auto *inData = reinterpret_cast<const uint8_t *>(bytes.data());
    if (! context.Process(&stream, inData, inSize)) {
        return std::unexpected(incerr_c::make(FONT_OTS_verificationProcessFailed));
    }

    auto outSizeOff = stream.Tell();
    if (outSizeOff < 0) { return std::unexpected(incerr_c::make(FONT_OTS_negativeOutputOffset)); }

    const auto outSize = static_cast<size_t>(outSizeOff);
    if (outSize == 0) { return std::unexpected(incerr_c::make(FONT_OTS_emptyOutput)); }

    std::vector<std::byte> sanitized(outSize);
    std::memcpy(sanitized.data(), stream.get(), outSize);
    return sanitized;
}

inline std::expected<std::vector<std::byte>, incerr_c> download_usingCPR(incplot::URI const &uri) {
    cpr::Session session;
    session.SetUrl(cpr::Url{uri.toString()});

    auto cb_writer = [](std::string_view data, intptr_t userdata) -> bool {
        std::vector<std::byte> *pf = reinterpret_cast<std::vector<std::byte> *>(userdata);
        auto v = std::views::transform(data, [](auto const &item) { return static_cast<std::byte>(item); });
        pf->insert(pf->end(), v.begin(), v.end());
        return true;
    };

    // TODO: Need to somehow report an error
    std::expected<std::vector<std::byte>, incerr_c> res = std::vector<std::byte>{};
    if (auto resLength = session.GetDownloadFileLength(); resLength > 0) {
        res.value().reserve(static_cast<size_t>(resLength));
    }
    else { res.value().reserve(4096 * 1024); }

    cpr::Response respo = session.Download(cpr::WriteCallback{cb_writer, reinterpret_cast<intptr_t>(&res)});
    if (respo.error.code != cpr::ErrorCode::OK) { res = std::unexpected(incerr_c::make(respo.error.code)); }

    return res;
}

std::expected<std::vector<DesiredPlot::DP_CtorStruct>, incerr_c> get_dpCtorStruct(argparse::ArgumentParser const &ap);
std::expected<std::vector<DesiredPlot::DP_CtorStruct>, incerr_c> get_dpCtorStruct();

std::expected<std::vector<std::string>, incerr_c> process_setupCommand(argparse::ArgumentParser const &setup_ap);

void finishAp(argparse::ArgumentParser &out_ap, argparse::ArgumentParser &subap_setup);
void populateAp(argparse::ArgumentParser &out_ap, int argc, const char *const *argv);

}; // namespace cl_args
} // namespace terminal_plot
} // namespace incom