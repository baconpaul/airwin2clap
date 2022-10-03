//
// Created by Paul Walker on 6/18/22.
//

#include "clap/clap.h"
#include "clap/helpers/plugin.hh"
#include "clap/helpers/plugin.hxx"

#include "clap/helpers/host-proxy.hh"
#include "clap/helpers/host-proxy.hxx"

#include "aw2c_configured.hxx"

#include <iostream>
#include <cassert>

struct aw2cWrapper : public clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate,
                                                     clap::helpers::CheckingLevel::Maximal>
{
    AudioEffect *underlyer{nullptr};
    aw2cWrapper(const clap_host *host, const clap_plugin_descriptor *desc, AudioEffect *effect)
        : clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate,
                                clap::helpers::CheckingLevel::Maximal>(desc, host),
                                underlyer(effect)
    {
        assert(underlyer);
    }
    ~aw2cWrapper() {
        delete underlyer;
    }

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
    void paramsFlush(const clap_input_events *ev, const clap_output_events *) noexcept override
    {
        auto sz = ev->size(ev);

        for (size_t i=0; i<sz; ++i)
        {
            auto *nextEvent = ev->get(ev, i);

            if (nextEvent->space_id == CLAP_CORE_EVENT_SPACE_ID)
            {
                switch (nextEvent->type)
                {
                case CLAP_EVENT_PARAM_VALUE:
                {
                    auto pevt = reinterpret_cast<const clap_event_param_value *>(nextEvent);
                    underlyer->setParameter(pevt->param_id - paramOff, pevt->value);
                    break;
                }
                default:
                    break;
                }
            }
        }
    }

    bool implementsAudioPorts() const noexcept override { return true; }
    uint32_t audioPortsCount(bool isInput) const noexcept override
    {
        // So VST is channels not ports so
        assert(underlyer->nin == 2 && underlyer->nout == 2);
        return 1;
    }
    bool audioPortsInfo(uint32_t index, bool isInput,
                        clap_audio_port_info *info) const noexcept override
    {
        info->id = isInput ? 2112 : 90210;
        info->in_place_pair = !isInput ? 2112 : 90210;
        strncpy(info->name, "main", sizeof(info->name));
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = 2;
        info->port_type = CLAP_PORT_STEREO;
        return true;
    }


    clap_process_status process(const clap_process *process) noexcept override
    {
        auto fc = process->frames_count;
        float *ip[2], *op[2];
        auto smp = 0U;

        auto ev = process->in_events;
        auto sz = ev->size(ev);

        // This pointer is the sentinel to our next event which we advance once an event is processed

        const clap_event_header_t *nextEvent{nullptr};
        uint32_t nextEventIndex{0};
        if (sz != 0)
        {
            nextEvent = ev->get(ev, nextEventIndex);
        }

        uint32_t basechunk = 32;
        while (smp < fc)
        {
            uint32_t chunk = std::min(basechunk, fc-smp);
            ip[0] = &process->audio_inputs->data32[0][smp];
            ip[1] = &process->audio_inputs->data32[1][smp];
            op[0] = &process->audio_outputs->data32[0][smp];
            op[1] = &process->audio_outputs->data32[1][smp];
            while (nextEvent && nextEvent->time < smp + chunk)
            {
                if (nextEvent->space_id == CLAP_CORE_EVENT_SPACE_ID)
                {
                    switch (nextEvent->type)
                    {
                    case CLAP_EVENT_PARAM_VALUE:
                    {
                        auto pevt = reinterpret_cast<const clap_event_param_value *>(nextEvent);
                        underlyer->setParameter(pevt->param_id - paramOff, pevt->value);
                        break;
                    }
                    default:
                        break;
                    }
                }

                nextEventIndex++;
                if (nextEventIndex >= sz)
                    nextEvent = nullptr;
                else
                    nextEvent = ev->get(ev, nextEventIndex);
            }

            underlyer->processReplacing(ip, op, chunk);
            smp += chunk;
        }

        assert (!nextEvent);
        return CLAP_PROCESS_CONTINUE;
    }

    bool implementsState() const noexcept override { return true; }
    bool stateSave(const clap_ostream *stream) noexcept override
    {
        uint8_t *data;
        auto s  = underlyer->getChunk((void**)&data, false);
        if (s && data)
        {
            auto c = data;
            while (s > 0)
            {
                auto r = stream->write(stream, c, s);
                if (r < 0)
                    return false;
                s -= r;
                c += r;
            }
            free(data);
        }

        // But no state is fine when we have no params
        if (!s && !data)
        {
            assert(paramsCount() == 0);
            if (paramsCount() > 0)
                return false;
        }
        return true;
    }
    bool stateLoad(const clap_istream *stream) noexcept override
    {
        char buffer[4096 * 16];
        char *bp = &(buffer[0]);
        int64_t rd;
        int total = 0;
        while ((rd = stream->read(stream, bp, 256)) > 0)
        {
            total += rd;
            bp += rd;
            if (total > 4096*16)
                return false;
        }

        underlyer->setChunk(buffer, total, false);
        return true;
    }
};

static const clap_plugin *clap_create_plugin(const clap_plugin_factory *f, const clap_host *host,
                                             const char *plugin_id)
{
    auto [fx, desc] = aw2c_get_aeffInstance(host, plugin_id);
    if (!fx)
        return nullptr;

    auto wr = new aw2cWrapper(host, desc, fx);
    return wr->clapPlugin();
}

const CLAP_EXPORT struct clap_plugin_factory aw2c_factory = {
    aw2c_get_plugin_count,
    aw2c_get_plugin_descriptor,
    clap_create_plugin,
};

static const void *get_factory(const char *factory_id) {
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0)
        return &aw2c_factory;
    return 0;
}

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
