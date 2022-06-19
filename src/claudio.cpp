//
// Created by Paul Walker on 6/18/22.
//

#include "clap/clap.h"
#include "clap/helpers/plugin.hh"
#include "clap/helpers/plugin.hxx"

#include "clap/helpers/host-proxy.hh"
#include "clap/helpers/host-proxy.hxx"

#include "claudio_configured.hxx"

#include <iostream>
#include <cassert>

struct ClaudioWrapper : public clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate,
                                                     clap::helpers::CheckingLevel::Maximal>
{
    AudioEffect *underlyer{nullptr};
    ClaudioWrapper(const clap_host *host, const clap_plugin_descriptor *desc, AudioEffect *effect)
        : clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate,
                                clap::helpers::CheckingLevel::Maximal>(desc, host),
                                underlyer(effect)
    {
       assert(underlyer);
    }
    ~ClaudioWrapper() = default;

    bool activate(double sampleRate, uint32_t minFrameCount,
                  uint32_t maxFrameCount) noexcept override
    {
        underlyer->setSampleRate(sampleRate);
        return true;
    }

  protected:
    static constexpr int paramOff = 10472;
    bool implementsParams() const noexcept override { return true; }
    bool isValidParamId(clap_id paramId) const noexcept override
    {
        return paramId >= paramOff && paramId < paramsCount() + paramOff;
    }
    uint32_t paramsCount() const noexcept override { return underlyer->numParams; }
    bool paramsInfo(uint32_t paramIndex, clap_param_info *info) const noexcept override
    {
        if (paramIndex > paramsCount())
            return false;

        info->flags = CLAP_PARAM_IS_AUTOMATABLE;
        info->id = paramIndex + paramOff;
        char tmp[CLAP_NAME_SIZE];
        underlyer->getParameterName(paramIndex, tmp);
        strncpy(info->name, tmp, CLAP_NAME_SIZE);
        info->module[0] = 0;
        info->min_value = 0;
        info->max_value = 1;
        info->default_value = underlyer->getParameter(paramIndex);
        return true;
    }
    bool paramsValue(clap_id paramId, double *value) noexcept override
    {
        *value = underlyer->getParameter(paramId - paramOff);
        return true;
    }
    bool paramsValueToText(clap_id paramId, double value, char *display,
                           uint32_t size) noexcept override
    {
        // I know this isn't right
        underlyer->getParameterDisplay(paramId - paramOff, display);
        return true;
    }

    bool implementsAudioPorts() const noexcept override { return true; }
    uint32_t audioPortsCount(bool isInput) const noexcept override
    {
        if (isInput) return underlyer->nin;
        return underlyer->nout;
    }
    bool audioPortsInfo(uint32_t index, bool isInput,
                        clap_audio_port_info *info) const noexcept override
    {
        info->id = isInput ? 2112 : 90210;
        strncpy(info->name, "main", sizeof(info->name));
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = 2;
        info->port_type = CLAP_PORT_STEREO;
        return true;
    }

    clap_process_status process(const clap_process *process) noexcept override
    {
        return Plugin::process(process);
    }
};

static const clap_plugin *clap_create_plugin(const clap_plugin_factory *f, const clap_host *host,
                                             const char *plugin_id)
{
    auto [fx, desc] = claudio_get_aeffInstance(host, plugin_id);
    std::cout << "FX = " << fx << " plugin_id = " << plugin_id << std::endl;

    auto wr = new ClaudioWrapper(host, desc, fx);
    return wr->clapPlugin();
}

const CLAP_EXPORT struct clap_plugin_factory claudio_factory = {
    claudio_get_plugin_count,
    claudio_get_plugin_descriptor,
    clap_create_plugin,
};

static const void *get_factory(const char *factory_id) { return &claudio_factory; }

// clap_init and clap_deinit are required to be fast, but we have nothing we need to do here
bool clap_init(const char *p) { return true; }

void clap_deinit() {}

extern "C"
{
    // clang-format off
const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
        CLAP_VERSION,
        clap_init,
        clap_deinit,
        get_factory
};
    // clang-format on
}
