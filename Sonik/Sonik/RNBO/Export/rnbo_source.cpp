/*******************************************************************************************************************
Copyright (c) 2023 Cycling '74

The code that Max generates automatically and that end users are capable of
exporting and using, and any associated documentation files (the “Software”)
is a work of authorship for which Cycling '74 is the author and owner for
copyright purposes.

This Software is dual-licensed either under the terms of the Cycling '74
License for Max-Generated Code for Export, or alternatively under the terms
of the General Public License (GPL) Version 3. You may use the Software
according to either of these licenses as it is most appropriate for your
project on a case-by-case basis (proprietary or not).

A) Cycling '74 License for Max-Generated Code for Export

A license is hereby granted, free of charge, to any person obtaining a copy
of the Software (“Licensee”) to use, copy, modify, merge, publish, and
distribute copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The Software is licensed to Licensee for all uses that do not include the sale,
sublicensing, or commercial distribution of software that incorporates this
source code. This means that the Licensee is free to use this software for
educational, research, and prototyping purposes, to create musical or other
creative works with software that incorporates this source code, or any other
use that does not constitute selling software that makes use of this source
code. Commercial distribution also includes the packaging of free software with
other paid software, hardware, or software-provided commercial services.

For entities with UNDER $200k in annual revenue or funding, a license is hereby
granted, free of charge, for the sale, sublicensing, or commercial distribution
of software that incorporates this source code, for as long as the entity's
annual revenue remains below $200k annual revenue or funding.

For entities with OVER $200k in annual revenue or funding interested in the
sale, sublicensing, or commercial distribution of software that incorporates
this source code, please send inquiries to licensing@cycling74.com.

The above copyright notice and this license shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Please see
https://support.cycling74.com/hc/en-us/articles/10730637742483-RNBO-Export-Licensing-FAQ
for additional information

B) General Public License Version 3 (GPLv3)
Details of the GPLv3 license can be found at: https://www.gnu.org/licenses/gpl-3.0.html
*******************************************************************************************************************/

#ifdef RNBO_LIB_PREFIX
#define STR_IMPL(A) #A
#define STR(A) STR_IMPL(A)
#define RNBO_LIB_INCLUDE(X) STR(RNBO_LIB_PREFIX/X)
#else
#define RNBO_LIB_INCLUDE(X) #X
#endif // RNBO_LIB_PREFIX
#ifdef RNBO_INJECTPLATFORM
#define RNBO_USECUSTOMPLATFORM
#include RNBO_INJECTPLATFORM
#endif // RNBO_INJECTPLATFORM

#include RNBO_LIB_INCLUDE(RNBO_Common.h)
#include RNBO_LIB_INCLUDE(RNBO_AudioSignal.h)

namespace RNBO {


#define trunc(x) ((Int)(x))
#define autoref auto&

#if defined(__GNUC__) || defined(__clang__)
    #define RNBO_RESTRICT __restrict__
#elif defined(_MSC_VER)
    #define RNBO_RESTRICT __restrict
#endif

#define FIXEDSIZEARRAYINIT(...) { }

template <class ENGINE = INTERNALENGINE> class rnbomatic : public PatcherInterfaceImpl {

friend class EngineCore;
friend class Engine;
friend class MinimalEngine<>;
public:

rnbomatic()
: _internalEngine(this)
{
}

~rnbomatic()
{
    deallocateSignals();
}

Index getNumMidiInputPorts() const {
    return 1;
}

void processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length) {
    this->updateTime(time, (ENGINE*)nullptr);
    this->midiin_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
    this->ctlin_17_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
    this->ctlin_18_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
    this->ctlin_19_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
    this->ctlin_20_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
}

Index getNumMidiOutputPorts() const {
    return 1;
}

void process(
    const SampleValue * const* inputs,
    Index numInputs,
    SampleValue * const* outputs,
    Index numOutputs,
    Index n
) {
    RNBO_UNUSED(numInputs);
    RNBO_UNUSED(inputs);
    this->vs = n;
    this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
    SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
    SampleValue * out2 = (numOutputs >= 2 && outputs[1] ? outputs[1] : this->dummyBuffer);
    SampleValue * out3 = (numOutputs >= 3 && outputs[2] ? outputs[2] : this->dummyBuffer);
    this->poly_perform(out1, out2, out3, n);
    this->stackprotect_perform(n);
    this->globaltransport_advance();
    this->advanceTime((ENGINE*)nullptr);
    this->audioProcessSampleCount += this->vs;
}

void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
    RNBO_ASSERT(this->_isInitialized);

    if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
        this->globaltransport_tempo = resizeSignal(this->globaltransport_tempo, this->maxvs, maxBlockSize);
        this->globaltransport_state = resizeSignal(this->globaltransport_state, this->maxvs, maxBlockSize);
        this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
        this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
        this->didAllocateSignals = true;
    }

    const bool sampleRateChanged = sampleRate != this->sr;
    const bool maxvsChanged = maxBlockSize != this->maxvs;
    const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;

    if (sampleRateChanged || maxvsChanged) {
        this->vs = maxBlockSize;
        this->maxvs = maxBlockSize;
        this->sr = sampleRate;
        this->invsr = 1 / sampleRate;
    }

    this->globaltransport_dspsetup(forceDSPSetup);

    for (Index i = 0; i < 4; i++) {
        this->poly[i]->prepareToProcess(sampleRate, maxBlockSize, force);
    }

    if (sampleRateChanged)
        this->onSampleRateChanged(sampleRate);
}

number msToSamps(MillisecondTime ms, number sampleRate) {
    return ms * sampleRate * 0.001;
}

MillisecondTime sampsToMs(SampleIndex samps) {
    return samps * (this->invsr * 1000);
}

Index getNumInputChannels() const {
    return 0;
}

Index getNumOutputChannels() const {
    return 3;
}

DataRef* getDataRef(DataRefIndex index)  {
    switch (index) {
    case 0:
        {
        return addressOf(this->RNBODefaultSinus);
        break;
        }
    case 1:
        {
        return addressOf(this->RNBODefaultMtofLookupTable256);
        break;
        }
    default:
        {
        return nullptr;
        }
    }
}

DataRefIndex getNumDataRefs() const {
    return 2;
}

void processDataViewUpdate(DataRefIndex index, MillisecondTime time) {
    for (Index i = 0; i < 4; i++) {
        this->poly[i]->processDataViewUpdate(index, time);
    }
}

void initialize() {
    RNBO_ASSERT(!this->_isInitialized);

    this->RNBODefaultSinus = initDataRef(
        this->RNBODefaultSinus,
        this->dataRefStrings->name0,
        true,
        this->dataRefStrings->file0,
        this->dataRefStrings->tag0
    );

    this->RNBODefaultMtofLookupTable256 = initDataRef(
        this->RNBODefaultMtofLookupTable256,
        this->dataRefStrings->name1,
        true,
        this->dataRefStrings->file1,
        this->dataRefStrings->tag1
    );

    this->assign_defaults();
    this->applyState();
    this->RNBODefaultSinus->setIndex(0);
    this->RNBODefaultMtofLookupTable256->setIndex(1);
    this->initializeObjects();
    this->allocateDataRefs();
    this->startup();
    this->_isInitialized = true;
}

void processTempoEvent(MillisecondTime time, Tempo tempo) {
    this->updateTime(time, (ENGINE*)nullptr);

    if (this->globaltransport_setTempo(this->_currentTime, tempo, false)) {
        for (Index i = 0; i < 4; i++) {
            this->poly[i]->processTempoEvent(time, tempo);
        }
    }
}

void processTransportEvent(MillisecondTime time, TransportState state) {
    this->updateTime(time, (ENGINE*)nullptr);

    if (this->globaltransport_setState(this->_currentTime, state, false)) {
        for (Index i = 0; i < 4; i++) {
            this->poly[i]->processTransportEvent(time, state);
        }
    }
}

void processBeatTimeEvent(MillisecondTime time, BeatTime beattime) {
    this->updateTime(time, (ENGINE*)nullptr);

    if (this->globaltransport_setBeatTime(this->_currentTime, beattime, false)) {
        for (Index i = 0; i < 4; i++) {
            this->poly[i]->processBeatTimeEvent(time, beattime);
        }
    }
}

void processTimeSignatureEvent(MillisecondTime time, Int numerator, Int denominator) {
    this->updateTime(time, (ENGINE*)nullptr);

    if (this->globaltransport_setTimeSignature(this->_currentTime, numerator, denominator, false)) {
        for (Index i = 0; i < 4; i++) {
            this->poly[i]->processTimeSignatureEvent(time, numerator, denominator);
        }
    }
}

void processBBUEvent(MillisecondTime time, number bars, number beats, number units) {
    this->updateTime(time, (ENGINE*)nullptr);

    if (this->globaltransport_setBBU(this->_currentTime, bars, beats, units, false)) {
        for (Index i = 0; i < 4; i++) {
            this->poly[i]->processBBUEvent(time, bars, beats, units);
        }
    }
}

void getPreset(PatcherStateInterface& preset) {
    this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
    preset["__presetid"] = "rnbo";
    this->param_19_getPresetValue(getSubState(preset, "harmonicity"));
    this->param_20_getPresetValue(getSubState(preset, "filter"));
    this->param_21_getPresetValue(getSubState(preset, "q"));
    this->param_22_getPresetValue(getSubState(preset, "mod_index"));
    this->param_23_getPresetValue(getSubState(preset, "mod_index_lfo_depth"));
    this->param_24_getPresetValue(getSubState(preset, "modi_index_lfo_freq"));

    for (Index i = 0; i < 4; i++)
        this->poly[i]->getPreset(getSubStateAt(getSubState(preset, "__sps"), "poly", i));
}

void setPreset(MillisecondTime time, PatcherStateInterface& preset) {
    this->updateTime(time, (ENGINE*)nullptr);

    for (Index i = 0; i < 4; i++)
        this->poly[i]->setPreset(time, getSubStateAt(getSubState(preset, "__sps"), "poly", i));

    this->param_19_setPresetValue(getSubState(preset, "harmonicity"));
    this->param_20_setPresetValue(getSubState(preset, "filter"));
    this->param_21_setPresetValue(getSubState(preset, "q"));
    this->param_22_setPresetValue(getSubState(preset, "mod_index"));
    this->param_23_setPresetValue(getSubState(preset, "mod_index_lfo_depth"));
    this->param_24_setPresetValue(getSubState(preset, "modi_index_lfo_freq"));

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_05->param_01_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "p_obj-18"), "attack")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_05->param_02_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "p_obj-18"), "decay")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_05->param_03_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "p_obj-18"), "sustain")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_05->param_04_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "p_obj-18"), "release")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_07->param_05_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "stereo_delay"), "left_delay")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_07->param_06_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "stereo_delay"), "fb")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_07->param_07_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "stereo_delay"), "right_delay")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_08->param_08_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "osc.analog[1]"), "lfotype")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_08->param_09_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "osc.analog[1]"), "on")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_08->param_10_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "osc.analog[1]"), "jitter")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_08->param_11_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "osc.analog[1]"), "smooth")
    );

    for (Index i1 = 0; i1 < 4; i1++) this->poly[i1]->p_08->param_12_setPresetValue(
        getSubState(getSubState(getSubState(getSubStateAt(getSubState(preset, "__sps"), "poly", i1), "__sps"), "osc.analog[1]"), "pulsewidth")
    );
}

void setParameterValue(ParameterIndex index, ParameterValue v, MillisecondTime time) {
    this->updateTime(time, (ENGINE*)nullptr);

    switch (index) {
    case 0:
        {
        this->param_19_value_set(v);
        break;
        }
    case 1:
        {
        this->param_20_value_set(v);
        break;
        }
    case 2:
        {
        this->param_21_value_set(v);
        break;
        }
    case 3:
        {
        this->param_22_value_set(v);
        break;
        }
    case 4:
        {
        this->param_23_value_set(v);
        break;
        }
    case 5:
        {
        this->param_24_value_set(v);
        break;
        }
    default:
        {
        index -= 6;

        if (index < this->poly[0]->getNumParameters())
            this->poly[0]->setPolyParameterValue(this->poly, index, v, time);

        break;
        }
    }
}

void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
    this->setParameterValue(index, value, time);
}

void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
    this->setParameterValue(index, this->getParameterValue(index), time);
}

void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
    this->setParameterValueNormalized(index, value, time);
}

ParameterValue getParameterValue(ParameterIndex index)  {
    switch (index) {
    case 0:
        {
        return this->param_19_value;
        }
    case 1:
        {
        return this->param_20_value;
        }
    case 2:
        {
        return this->param_21_value;
        }
    case 3:
        {
        return this->param_22_value;
        }
    case 4:
        {
        return this->param_23_value;
        }
    case 5:
        {
        return this->param_24_value;
        }
    default:
        {
        index -= 6;

        if (index < this->poly[0]->getNumParameters())
            return this->poly[0]->getPolyParameterValue(this->poly, index);

        return 0;
        }
    }
}

ParameterIndex getNumSignalInParameters() const {
    return 0;
}

ParameterIndex getNumSignalOutParameters() const {
    return 0;
}

ParameterIndex getNumParameters() const {
    return 6 + this->poly[0]->getNumParameters();
}

ConstCharPointer getParameterName(ParameterIndex index) const {
    switch (index) {
    case 0:
        {
        return "harmonicity";
        }
    case 1:
        {
        return "filter";
        }
    case 2:
        {
        return "q";
        }
    case 3:
        {
        return "mod_index";
        }
    case 4:
        {
        return "mod_index_lfo_depth";
        }
    case 5:
        {
        return "modi_index_lfo_freq";
        }
    default:
        {
        index -= 6;

        if (index < this->poly[0]->getNumParameters()) {
            {
                return this->poly[0]->getParameterName(index);
            }
        }

        return "bogus";
        }
    }
}

ConstCharPointer getParameterId(ParameterIndex index) const {
    switch (index) {
    case 0:
        {
        return "harmonicity";
        }
    case 1:
        {
        return "filter";
        }
    case 2:
        {
        return "q";
        }
    case 3:
        {
        return "mod_index";
        }
    case 4:
        {
        return "mod_index_lfo_depth";
        }
    case 5:
        {
        return "modi_index_lfo_freq";
        }
    default:
        {
        index -= 6;

        if (index < this->poly[0]->getNumParameters()) {
            {
                return this->poly[0]->getParameterId(index);
            }
        }

        return "bogus";
        }
    }
}

void getParameterInfo(ParameterIndex index, ParameterInfo * info) const {
    {
        switch (index) {
        case 0:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 1;
            info->min = 1;
            info->max = 10;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 1:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 12000;
            info->min = 20;
            info->max = 20480;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 2:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 1;
            info->min = 0.1;
            info->max = 4;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 3:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 1;
            info->min = 0.1;
            info->max = 10;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 4:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 10;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 5:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0.2;
            info->min = 0;
            info->max = 40;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        default:
            {
            index -= 6;

            if (index < this->poly[0]->getNumParameters()) {
                for (Index i = 0; i < 4; i++) {
                    this->poly[i]->getParameterInfo(index, info);
                }
            }

            break;
            }
        }
    }
}

ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
    if (steps == 1) {
        if (normalizedValue > 0) {
            normalizedValue = 1.;
        }
    } else {
        ParameterValue oneStep = (number)1. / (steps - 1);
        ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
        normalizedValue = numberOfSteps * oneStep;
    }

    return normalizedValue;
}

ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
    switch (index) {
    case 4:
        {
        {
            value = (value < 0 ? 0 : (value > 10 ? 10 : value));
            ParameterValue normalizedValue = (value - 0) / (10 - 0);
            return normalizedValue;
        }
        }
    case 5:
        {
        {
            value = (value < 0 ? 0 : (value > 40 ? 40 : value));
            ParameterValue normalizedValue = (value - 0) / (40 - 0);
            return normalizedValue;
        }
        }
    case 0:
        {
        {
            value = (value < 1 ? 1 : (value > 10 ? 10 : value));
            ParameterValue normalizedValue = (value - 1) / (10 - 1);
            return normalizedValue;
        }
        }
    case 1:
        {
        return value = rnbo_log2((value / (number)20 <= 0.0000000001 ? 0.0000000001 : value / (number)20)) / (number)10, (value <= 0.0 ? 0.0 : (value >= 1.0 ? 1.0 : value));
        }
    case 2:
        {
        {
            value = (value < 0.1 ? 0.1 : (value > 4 ? 4 : value));
            ParameterValue normalizedValue = (value - 0.1) / (4 - 0.1);
            return normalizedValue;
        }
        }
    case 3:
        {
        {
            value = (value < 0.1 ? 0.1 : (value > 10 ? 10 : value));
            ParameterValue normalizedValue = (value - 0.1) / (10 - 0.1);
            return normalizedValue;
        }
        }
    default:
        {
        index -= 6;

        if (index < this->poly[0]->getNumParameters()) {
            {
                return this->poly[0]->convertToNormalizedParameterValue(index, value);
            }
        }

        return value;
        }
    }
}

ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
    value = (value < 0 ? 0 : (value > 1 ? 1 : value));

    switch (index) {
    case 4:
        {
        {
            {
                return 0 + value * (10 - 0);
            }
        }
        }
    case 5:
        {
        {
            {
                return 0 + value * (40 - 0);
            }
        }
        }
    case 0:
        {
        {
            {
                return 1 + value * (10 - 1);
            }
        }
        }
    case 1:
        {
        {
            {
                return 20 + value * (20480 - 20);
            }
        }
        }
    case 2:
        {
        {
            {
                return 0.1 + value * (4 - 0.1);
            }
        }
        }
    case 3:
        {
        {
            {
                return 0.1 + value * (10 - 0.1);
            }
        }
        }
    default:
        {
        index -= 6;

        if (index < this->poly[0]->getNumParameters()) {
            {
                return this->poly[0]->convertFromNormalizedParameterValue(index, value);
            }
        }

        return value;
        }
    }
}

ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
    switch (index) {
    case 0:
        {
        return this->param_19_value_constrain(value);
        }
    case 1:
        {
        return this->param_20_value_constrain(value);
        }
    case 2:
        {
        return this->param_21_value_constrain(value);
        }
    case 3:
        {
        return this->param_22_value_constrain(value);
        }
    case 4:
        {
        return this->param_23_value_constrain(value);
        }
    case 5:
        {
        return this->param_24_value_constrain(value);
        }
    default:
        {
        index -= 6;

        if (index < this->poly[0]->getNumParameters()) {
            {
                return this->poly[0]->constrainParameterValue(index, value);
            }
        }

        return value;
        }
    }
}

void processNumMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, number payload) {
    RNBO_UNUSED(objectId);
    this->updateTime(time, (ENGINE*)nullptr);

    for (Index i = 0; i < 4; i++) {
        this->poly[i]->processNumMessage(tag, objectId, time, payload);
    }
}

void processListMessage(
    MessageTag tag,
    MessageTag objectId,
    MillisecondTime time,
    const list& payload
) {
    RNBO_UNUSED(objectId);
    this->updateTime(time, (ENGINE*)nullptr);

    for (Index i = 0; i < 4; i++) {
        this->poly[i]->processListMessage(tag, objectId, time, payload);
    }
}

void processBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) {
    RNBO_UNUSED(objectId);
    this->updateTime(time, (ENGINE*)nullptr);

    for (Index i = 0; i < 4; i++) {
        this->poly[i]->processBangMessage(tag, objectId, time);
    }
}

MessageTagInfo resolveTag(MessageTag tag) const {
    switch (tag) {

    }

    auto subpatchResult_0 = this->poly[0]->resolveTag(tag);

    if (subpatchResult_0)
        return subpatchResult_0;

    return "";
}

MessageIndex getNumMessages() const {
    return 0;
}

const MessageInfo& getMessageInfo(MessageIndex index) const {
    switch (index) {

    }

    return NullMessageInfo;
}

protected:

class RNBOSubpatcher_953 : public PatcherInterfaceImpl {
    
    friend class rnbomatic;
    
    public:
    
    RNBOSubpatcher_953()
    {}
    
    ~RNBOSubpatcher_953()
    {
        deallocateSignals();
    }
    
    Index getNumMidiInputPorts() const {
        return 1;
    }
    
    void processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length) {
        this->updateTime(time, (ENGINE*)nullptr);
        this->p_05_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
        this->notein_01_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
        this->p_07_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
        this->p_08_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
        this->ctlin_13_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
        this->ctlin_14_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
        this->ctlin_15_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
        this->ctlin_16_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
    }
    
    Index getNumMidiOutputPorts() const {
        return 0;
    }
    
    void process(
        const SampleValue * const* inputs,
        Index numInputs,
        SampleValue * const* outputs,
        Index numOutputs,
        Index n
    ) {
        RNBO_UNUSED(numInputs);
        RNBO_UNUSED(inputs);
        this->vs = n;
        this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
        SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
        SampleValue * out2 = (numOutputs >= 2 && outputs[1] ? outputs[1] : this->dummyBuffer);
        SampleValue * out3 = (numOutputs >= 3 && outputs[2] ? outputs[2] : this->dummyBuffer);
    
        if (this->getIsMuted())
            return;
    
        this->mtof_tilde_01_perform(this->mtof_tilde_01_midivalue, this->signals[0], n);
    
        this->slide_tilde_03_perform(
            this->slide_tilde_03_x,
            this->slide_tilde_03_up,
            this->slide_tilde_03_down,
            this->signals[1],
            n
        );
    
        this->slide_tilde_04_perform(
            this->slide_tilde_04_x,
            this->slide_tilde_04_up,
            this->slide_tilde_04_down,
            this->signals[2],
            n
        );
    
        this->slide_tilde_07_perform(
            this->slide_tilde_07_x,
            this->slide_tilde_07_up,
            this->slide_tilde_07_down,
            this->signals[3],
            n
        );
    
        this->filtercoeff_tilde_01_perform(
            this->signals[2],
            this->filtercoeff_tilde_01_gain,
            this->signals[3],
            this->signals[4],
            this->signals[5],
            this->signals[6],
            this->signals[7],
            this->signals[8],
            n
        );
    
        this->slide_tilde_05_perform(
            this->signals[4],
            this->slide_tilde_05_up,
            this->slide_tilde_05_down,
            this->signals[3],
            n
        );
    
        this->slide_tilde_06_perform(
            this->signals[5],
            this->slide_tilde_06_up,
            this->slide_tilde_06_down,
            this->signals[4],
            n
        );
    
        this->slide_tilde_08_perform(
            this->signals[6],
            this->slide_tilde_08_up,
            this->slide_tilde_08_down,
            this->signals[5],
            n
        );
    
        this->slide_tilde_09_perform(
            this->signals[7],
            this->slide_tilde_09_up,
            this->slide_tilde_09_down,
            this->signals[6],
            n
        );
    
        this->slide_tilde_11_perform(
            this->signals[8],
            this->slide_tilde_11_up,
            this->slide_tilde_11_down,
            this->signals[7],
            n
        );
    
        this->ip_08_perform(this->signals[8], n);
    
        this->slide_tilde_10_perform(
            this->slide_tilde_10_x,
            this->slide_tilde_10_up,
            this->slide_tilde_10_down,
            this->signals[2],
            n
        );
    
        this->slide_tilde_12_perform(
            this->slide_tilde_12_x,
            this->slide_tilde_12_up,
            this->slide_tilde_12_down,
            this->signals[9],
            n
        );
    
        this->p_08_perform(this->signals[9], this->signals[10], n);
    
        this->slide_tilde_13_perform(
            this->slide_tilde_13_x,
            this->slide_tilde_13_up,
            this->slide_tilde_13_down,
            this->signals[9],
            n
        );
    
        this->dspexpr_20_perform(this->signals[10], this->signals[9], this->signals[11], n);
        this->dspexpr_19_perform(this->signals[2], this->signals[11], this->signals[9], n);
    
        this->dspexpr_18_perform(
            this->signals[9],
            this->dspexpr_18_in2,
            this->dspexpr_18_in3,
            this->signals[11],
            n
        );
    
        this->signaladder_01_perform(this->signals[11], out3, out3, n);
    
        this->p_06_perform(
            this->signals[0],
            this->signals[1],
            this->signals[11],
            this->signals[9],
            this->dummyBuffer,
            n
        );
    
        this->p_05_perform(
            this->signals[9],
            this->signals[8],
            this->signals[11],
            this->dummyBuffer,
            n
        );
    
        this->biquad_tilde_01_perform(
            this->signals[11],
            this->signals[3],
            this->signals[4],
            this->signals[5],
            this->signals[6],
            this->signals[7],
            this->signals[8],
            n
        );
    
        this->p_07_perform(this->signals[8], this->signals[7], this->signals[6], n);
        this->signaladder_02_perform(this->signals[7], out1, out1, n);
        this->signaladder_03_perform(this->signals[6], out2, out2, n);
        this->stackprotect_perform(n);
        this->audioProcessSampleCount += this->vs;
    }
    
    void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
        RNBO_ASSERT(this->_isInitialized);
    
        if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
            Index i;
    
            for (i = 0; i < 12; i++) {
                this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
            }
    
            this->ip_08_sigbuf = resizeSignal(this->ip_08_sigbuf, this->maxvs, maxBlockSize);
            this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
            this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
            this->didAllocateSignals = true;
        }
    
        const bool sampleRateChanged = sampleRate != this->sr;
        const bool maxvsChanged = maxBlockSize != this->maxvs;
        const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;
    
        if (sampleRateChanged || maxvsChanged) {
            this->vs = maxBlockSize;
            this->maxvs = maxBlockSize;
            this->sr = sampleRate;
            this->invsr = 1 / sampleRate;
        }
    
        this->filtercoeff_tilde_01_dspsetup(forceDSPSetup);
        this->ip_08_dspsetup(forceDSPSetup);
        this->biquad_tilde_01_dspsetup(forceDSPSetup);
        this->p_05->prepareToProcess(sampleRate, maxBlockSize, force);
        this->p_06->prepareToProcess(sampleRate, maxBlockSize, force);
        this->p_07->prepareToProcess(sampleRate, maxBlockSize, force);
        this->p_08->prepareToProcess(sampleRate, maxBlockSize, force);
    
        if (sampleRateChanged)
            this->onSampleRateChanged(sampleRate);
    }
    
    number msToSamps(MillisecondTime ms, number sampleRate) {
        return ms * sampleRate * 0.001;
    }
    
    MillisecondTime sampsToMs(SampleIndex samps) {
        return samps * (this->invsr * 1000);
    }
    
    Index getNumInputChannels() const {
        return 0;
    }
    
    Index getNumOutputChannels() const {
        return 3;
    }
    
    void processTempoEvent(MillisecondTime time, Tempo tempo) {
        this->updateTime(time, (ENGINE*)nullptr);
        this->p_05->processTempoEvent(time, tempo);
        this->p_06->processTempoEvent(time, tempo);
        this->p_07->processTempoEvent(time, tempo);
        this->p_08->processTempoEvent(time, tempo);
    }
    
    void processTransportEvent(MillisecondTime time, TransportState state) {
        this->updateTime(time, (ENGINE*)nullptr);
        this->p_05->processTransportEvent(time, state);
        this->p_06->processTransportEvent(time, state);
        this->p_07->processTransportEvent(time, state);
        this->p_08->processTransportEvent(time, state);
    }
    
    void processBeatTimeEvent(MillisecondTime time, BeatTime beattime) {
        this->updateTime(time, (ENGINE*)nullptr);
        this->p_05->processBeatTimeEvent(time, beattime);
        this->p_06->processBeatTimeEvent(time, beattime);
        this->p_07->processBeatTimeEvent(time, beattime);
        this->p_08->processBeatTimeEvent(time, beattime);
    }
    
    void processTimeSignatureEvent(MillisecondTime time, Int numerator, Int denominator) {
        this->updateTime(time, (ENGINE*)nullptr);
        this->p_05->processTimeSignatureEvent(time, numerator, denominator);
        this->p_06->processTimeSignatureEvent(time, numerator, denominator);
        this->p_07->processTimeSignatureEvent(time, numerator, denominator);
        this->p_08->processTimeSignatureEvent(time, numerator, denominator);
    }
    
    void processBBUEvent(MillisecondTime time, number bars, number beats, number units) {
        this->updateTime(time, (ENGINE*)nullptr);
        this->p_05->processBBUEvent(time, bars, beats, units);
        this->p_06->processBBUEvent(time, bars, beats, units);
        this->p_07->processBBUEvent(time, bars, beats, units);
        this->p_08->processBBUEvent(time, bars, beats, units);
    }
    
    void getPreset(PatcherStateInterface& preset) {
        this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
        this->p_05->getPreset(getSubState(getSubState(preset, "__sps"), "p_obj-18"));
        this->p_06->getPreset(getSubState(getSubState(preset, "__sps"), "osc.fm"));
        this->p_07->getPreset(getSubState(getSubState(preset, "__sps"), "stereo_delay"));
        this->p_08->getPreset(getSubState(getSubState(preset, "__sps"), "osc.analog[1]"));
    }
    
    void setPreset(MillisecondTime time, PatcherStateInterface& preset) {
        this->p_05->setPreset(time, getSubState(getSubState(preset, "__sps"), "p_obj-18"));
        this->p_06->setPreset(time, getSubState(getSubState(preset, "__sps"), "osc.fm"));
        this->p_07->setPreset(time, getSubState(getSubState(preset, "__sps"), "stereo_delay"));
        this->p_08->setPreset(time, getSubState(getSubState(preset, "__sps"), "osc.analog[1]"));
    }
    
    void setParameterValue(ParameterIndex index, ParameterValue v, MillisecondTime time) {
        this->updateTime(time, (ENGINE*)nullptr);
    
        switch (index) {
        case 0:
            {
            this->param_13_value_set(v);
            break;
            }
        case 1:
            {
            this->param_14_value_set(v);
            break;
            }
        case 2:
            {
            this->param_15_value_set(v);
            break;
            }
        case 3:
            {
            this->param_16_value_set(v);
            break;
            }
        case 4:
            {
            this->param_17_value_set(v);
            break;
            }
        case 5:
            {
            this->param_18_value_set(v);
            break;
            }
        default:
            {
            index -= 6;
    
            if (index < this->p_05->getNumParameters())
                this->p_05->setParameterValue(index, v, time);
    
            index -= this->p_05->getNumParameters();
    
            if (index < this->p_06->getNumParameters())
                this->p_06->setParameterValue(index, v, time);
    
            index -= this->p_06->getNumParameters();
    
            if (index < this->p_07->getNumParameters())
                this->p_07->setParameterValue(index, v, time);
    
            index -= this->p_07->getNumParameters();
    
            if (index < this->p_08->getNumParameters())
                this->p_08->setParameterValue(index, v, time);
    
            break;
            }
        }
    }
    
    void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
        this->setParameterValue(index, value, time);
    }
    
    void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
        this->setParameterValue(index, this->getParameterValue(index), time);
    }
    
    void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
        this->setParameterValueNormalized(index, value, time);
    }
    
    ParameterValue getParameterValue(ParameterIndex index)  {
        switch (index) {
        case 0:
            {
            return this->param_13_value;
            }
        case 1:
            {
            return this->param_14_value;
            }
        case 2:
            {
            return this->param_15_value;
            }
        case 3:
            {
            return this->param_16_value;
            }
        case 4:
            {
            return this->param_17_value;
            }
        case 5:
            {
            return this->param_18_value;
            }
        default:
            {
            index -= 6;
    
            if (index < this->p_05->getNumParameters())
                return this->p_05->getParameterValue(index);
    
            index -= this->p_05->getNumParameters();
    
            if (index < this->p_06->getNumParameters())
                return this->p_06->getParameterValue(index);
    
            index -= this->p_06->getNumParameters();
    
            if (index < this->p_07->getNumParameters())
                return this->p_07->getParameterValue(index);
    
            index -= this->p_07->getNumParameters();
    
            if (index < this->p_08->getNumParameters())
                return this->p_08->getParameterValue(index);
    
            return 0;
            }
        }
    }
    
    ParameterIndex getNumSignalInParameters() const {
        return 0;
    }
    
    ParameterIndex getNumSignalOutParameters() const {
        return 0;
    }
    
    ParameterIndex getNumParameters() const {
        return 6 + this->p_05->getNumParameters() + this->p_06->getNumParameters() + this->p_07->getNumParameters() + this->p_08->getNumParameters();
    }
    
    ConstCharPointer getParameterName(ParameterIndex index) const {
        switch (index) {
        case 0:
            {
            return "harmonicity";
            }
        case 1:
            {
            return "filter";
            }
        case 2:
            {
            return "q";
            }
        case 3:
            {
            return "mod_index";
            }
        case 4:
            {
            return "mod_index_lfo_depth";
            }
        case 5:
            {
            return "modi_index_lfo_freq";
            }
        default:
            {
            index -= 6;
    
            if (index < this->p_05->getNumParameters())
                return this->p_05->getParameterName(index);
    
            index -= this->p_05->getNumParameters();
    
            if (index < this->p_06->getNumParameters())
                return this->p_06->getParameterName(index);
    
            index -= this->p_06->getNumParameters();
    
            if (index < this->p_07->getNumParameters())
                return this->p_07->getParameterName(index);
    
            index -= this->p_07->getNumParameters();
    
            if (index < this->p_08->getNumParameters())
                return this->p_08->getParameterName(index);
    
            return "bogus";
            }
        }
    }
    
    ConstCharPointer getParameterId(ParameterIndex index) const {
        switch (index) {
        case 0:
            {
            return "poly/harmonicity";
            }
        case 1:
            {
            return "poly/filter";
            }
        case 2:
            {
            return "poly/q";
            }
        case 3:
            {
            return "poly/mod_index";
            }
        case 4:
            {
            return "poly/mod_index_lfo_depth";
            }
        case 5:
            {
            return "poly/modi_index_lfo_freq";
            }
        default:
            {
            index -= 6;
    
            if (index < this->p_05->getNumParameters())
                return this->p_05->getParameterId(index);
    
            index -= this->p_05->getNumParameters();
    
            if (index < this->p_06->getNumParameters())
                return this->p_06->getParameterId(index);
    
            index -= this->p_06->getNumParameters();
    
            if (index < this->p_07->getNumParameters())
                return this->p_07->getParameterId(index);
    
            index -= this->p_07->getNumParameters();
    
            if (index < this->p_08->getNumParameters())
                return this->p_08->getParameterId(index);
    
            return "bogus";
            }
        }
    }
    
    void getParameterInfo(ParameterIndex index, ParameterInfo * info) const {
        {
            switch (index) {
            case 0:
                {
                info->type = ParameterTypeNumber;
                info->initialValue = 1;
                info->min = 1;
                info->max = 10;
                info->exponent = 1;
                info->steps = 0;
                info->debug = false;
                info->saveable = true;
                info->transmittable = true;
                info->initialized = true;
                info->visible = false;
                info->displayName = "";
                info->unit = "";
                info->ioType = IOTypeUndefined;
                info->signalIndex = INVALID_INDEX;
                break;
                }
            case 1:
                {
                info->type = ParameterTypeNumber;
                info->initialValue = 12000;
                info->min = 20;
                info->max = 20480;
                info->exponent = 1;
                info->steps = 0;
                info->debug = false;
                info->saveable = true;
                info->transmittable = true;
                info->initialized = true;
                info->visible = false;
                info->displayName = "";
                info->unit = "";
                info->ioType = IOTypeUndefined;
                info->signalIndex = INVALID_INDEX;
                break;
                }
            case 2:
                {
                info->type = ParameterTypeNumber;
                info->initialValue = 1;
                info->min = 0.1;
                info->max = 4;
                info->exponent = 1;
                info->steps = 0;
                info->debug = false;
                info->saveable = true;
                info->transmittable = true;
                info->initialized = true;
                info->visible = false;
                info->displayName = "";
                info->unit = "";
                info->ioType = IOTypeUndefined;
                info->signalIndex = INVALID_INDEX;
                break;
                }
            case 3:
                {
                info->type = ParameterTypeNumber;
                info->initialValue = 1;
                info->min = 0.1;
                info->max = 10;
                info->exponent = 1;
                info->steps = 0;
                info->debug = false;
                info->saveable = true;
                info->transmittable = true;
                info->initialized = true;
                info->visible = false;
                info->displayName = "";
                info->unit = "";
                info->ioType = IOTypeUndefined;
                info->signalIndex = INVALID_INDEX;
                break;
                }
            case 4:
                {
                info->type = ParameterTypeNumber;
                info->initialValue = 0;
                info->min = 0;
                info->max = 10;
                info->exponent = 1;
                info->steps = 0;
                info->debug = false;
                info->saveable = true;
                info->transmittable = true;
                info->initialized = true;
                info->visible = false;
                info->displayName = "";
                info->unit = "";
                info->ioType = IOTypeUndefined;
                info->signalIndex = INVALID_INDEX;
                break;
                }
            case 5:
                {
                info->type = ParameterTypeNumber;
                info->initialValue = 0.2;
                info->min = 0;
                info->max = 40;
                info->exponent = 1;
                info->steps = 0;
                info->debug = false;
                info->saveable = true;
                info->transmittable = true;
                info->initialized = true;
                info->visible = false;
                info->displayName = "";
                info->unit = "";
                info->ioType = IOTypeUndefined;
                info->signalIndex = INVALID_INDEX;
                break;
                }
            default:
                {
                index -= 6;
    
                if (index < this->p_05->getNumParameters())
                    this->p_05->getParameterInfo(index, info);
    
                index -= this->p_05->getNumParameters();
    
                if (index < this->p_06->getNumParameters())
                    this->p_06->getParameterInfo(index, info);
    
                index -= this->p_06->getNumParameters();
    
                if (index < this->p_07->getNumParameters())
                    this->p_07->getParameterInfo(index, info);
    
                index -= this->p_07->getNumParameters();
    
                if (index < this->p_08->getNumParameters())
                    this->p_08->getParameterInfo(index, info);
    
                break;
                }
            }
        }
    }
    
    ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
        if (steps == 1) {
            if (normalizedValue > 0) {
                normalizedValue = 1.;
            }
        } else {
            ParameterValue oneStep = (number)1. / (steps - 1);
            ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
            normalizedValue = numberOfSteps * oneStep;
        }
    
        return normalizedValue;
    }
    
    ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
        switch (index) {
        case 4:
            {
            {
                value = (value < 0 ? 0 : (value > 10 ? 10 : value));
                ParameterValue normalizedValue = (value - 0) / (10 - 0);
                return normalizedValue;
            }
            }
        case 5:
            {
            {
                value = (value < 0 ? 0 : (value > 40 ? 40 : value));
                ParameterValue normalizedValue = (value - 0) / (40 - 0);
                return normalizedValue;
            }
            }
        case 0:
            {
            {
                value = (value < 1 ? 1 : (value > 10 ? 10 : value));
                ParameterValue normalizedValue = (value - 1) / (10 - 1);
                return normalizedValue;
            }
            }
        case 1:
            {
            return value = rnbo_log2((value / (number)20 <= 0.0000000001 ? 0.0000000001 : value / (number)20)) / (number)10, (value <= 0.0 ? 0.0 : (value >= 1.0 ? 1.0 : value));
            }
        case 2:
            {
            {
                value = (value < 0.1 ? 0.1 : (value > 4 ? 4 : value));
                ParameterValue normalizedValue = (value - 0.1) / (4 - 0.1);
                return normalizedValue;
            }
            }
        case 3:
            {
            {
                value = (value < 0.1 ? 0.1 : (value > 10 ? 10 : value));
                ParameterValue normalizedValue = (value - 0.1) / (10 - 0.1);
                return normalizedValue;
            }
            }
        default:
            {
            index -= 6;
    
            if (index < this->p_05->getNumParameters())
                return this->p_05->convertToNormalizedParameterValue(index, value);
    
            index -= this->p_05->getNumParameters();
    
            if (index < this->p_06->getNumParameters())
                return this->p_06->convertToNormalizedParameterValue(index, value);
    
            index -= this->p_06->getNumParameters();
    
            if (index < this->p_07->getNumParameters())
                return this->p_07->convertToNormalizedParameterValue(index, value);
    
            index -= this->p_07->getNumParameters();
    
            if (index < this->p_08->getNumParameters())
                return this->p_08->convertToNormalizedParameterValue(index, value);
    
            return value;
            }
        }
    }
    
    ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
        value = (value < 0 ? 0 : (value > 1 ? 1 : value));
    
        switch (index) {
        case 4:
            {
            {
                {
                    return 0 + value * (10 - 0);
                }
            }
            }
        case 5:
            {
            {
                {
                    return 0 + value * (40 - 0);
                }
            }
            }
        case 0:
            {
            {
                {
                    return 1 + value * (10 - 1);
                }
            }
            }
        case 1:
            {
            {
                {
                    return 20 + value * (20480 - 20);
                }
            }
            }
        case 2:
            {
            {
                {
                    return 0.1 + value * (4 - 0.1);
                }
            }
            }
        case 3:
            {
            {
                {
                    return 0.1 + value * (10 - 0.1);
                }
            }
            }
        default:
            {
            index -= 6;
    
            if (index < this->p_05->getNumParameters())
                return this->p_05->convertFromNormalizedParameterValue(index, value);
    
            index -= this->p_05->getNumParameters();
    
            if (index < this->p_06->getNumParameters())
                return this->p_06->convertFromNormalizedParameterValue(index, value);
    
            index -= this->p_06->getNumParameters();
    
            if (index < this->p_07->getNumParameters())
                return this->p_07->convertFromNormalizedParameterValue(index, value);
    
            index -= this->p_07->getNumParameters();
    
            if (index < this->p_08->getNumParameters())
                return this->p_08->convertFromNormalizedParameterValue(index, value);
    
            return value;
            }
        }
    }
    
    ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
        switch (index) {
        case 0:
            {
            return this->param_13_value_constrain(value);
            }
        case 1:
            {
            return this->param_14_value_constrain(value);
            }
        case 2:
            {
            return this->param_15_value_constrain(value);
            }
        case 3:
            {
            return this->param_16_value_constrain(value);
            }
        case 4:
            {
            return this->param_17_value_constrain(value);
            }
        case 5:
            {
            return this->param_18_value_constrain(value);
            }
        default:
            {
            index -= 6;
    
            if (index < this->p_05->getNumParameters())
                return this->p_05->constrainParameterValue(index, value);
    
            index -= this->p_05->getNumParameters();
    
            if (index < this->p_06->getNumParameters())
                return this->p_06->constrainParameterValue(index, value);
    
            index -= this->p_06->getNumParameters();
    
            if (index < this->p_07->getNumParameters())
                return this->p_07->constrainParameterValue(index, value);
    
            index -= this->p_07->getNumParameters();
    
            if (index < this->p_08->getNumParameters())
                return this->p_08->constrainParameterValue(index, value);
    
            return value;
            }
        }
    }
    
    void processNumMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, number payload) {
        this->updateTime(time, (ENGINE*)nullptr);
    
        switch (tag) {
        case TAG("valin"):
            {
            if (TAG("poly/number_obj-44") == objectId)
                this->numberobj_01_valin_set(payload);
    
            if (TAG("poly/number_obj-34") == objectId)
                this->numberobj_02_valin_set(payload);
    
            if (TAG("poly/number_obj-31") == objectId)
                this->numberobj_03_valin_set(payload);
    
            if (TAG("poly/number_obj-41") == objectId)
                this->numberobj_04_valin_set(payload);
    
            break;
            }
        case TAG("format"):
            {
            if (TAG("poly/number_obj-44") == objectId)
                this->numberobj_01_format_set(payload);
    
            if (TAG("poly/number_obj-34") == objectId)
                this->numberobj_02_format_set(payload);
    
            if (TAG("poly/number_obj-31") == objectId)
                this->numberobj_03_format_set(payload);
    
            if (TAG("poly/number_obj-41") == objectId)
                this->numberobj_04_format_set(payload);
    
            break;
            }
        }
    
        this->p_05->processNumMessage(tag, objectId, time, payload);
        this->p_06->processNumMessage(tag, objectId, time, payload);
        this->p_07->processNumMessage(tag, objectId, time, payload);
        this->p_08->processNumMessage(tag, objectId, time, payload);
    }
    
    void processListMessage(
        MessageTag tag,
        MessageTag objectId,
        MillisecondTime time,
        const list& payload
    ) {
        RNBO_UNUSED(objectId);
        this->updateTime(time, (ENGINE*)nullptr);
        this->p_05->processListMessage(tag, objectId, time, payload);
        this->p_06->processListMessage(tag, objectId, time, payload);
        this->p_07->processListMessage(tag, objectId, time, payload);
        this->p_08->processListMessage(tag, objectId, time, payload);
    }
    
    void processBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) {
        RNBO_UNUSED(objectId);
        this->updateTime(time, (ENGINE*)nullptr);
        this->p_05->processBangMessage(tag, objectId, time);
        this->p_06->processBangMessage(tag, objectId, time);
        this->p_07->processBangMessage(tag, objectId, time);
        this->p_08->processBangMessage(tag, objectId, time);
    }
    
    MessageTagInfo resolveTag(MessageTag tag) const {
        switch (tag) {
        case TAG("valout"):
            {
            return "valout";
            }
        case TAG("poly/number_obj-44"):
            {
            return "poly/number_obj-44";
            }
        case TAG("setup"):
            {
            return "setup";
            }
        case TAG("poly/number_obj-34"):
            {
            return "poly/number_obj-34";
            }
        case TAG("poly/number_obj-31"):
            {
            return "poly/number_obj-31";
            }
        case TAG("poly/number_obj-41"):
            {
            return "poly/number_obj-41";
            }
        case TAG("valin"):
            {
            return "valin";
            }
        case TAG("format"):
            {
            return "format";
            }
        }
    
        auto subpatchResult_0 = this->p_05->resolveTag(tag);
    
        if (subpatchResult_0)
            return subpatchResult_0;
    
        auto subpatchResult_1 = this->p_06->resolveTag(tag);
    
        if (subpatchResult_1)
            return subpatchResult_1;
    
        auto subpatchResult_2 = this->p_07->resolveTag(tag);
    
        if (subpatchResult_2)
            return subpatchResult_2;
    
        auto subpatchResult_3 = this->p_08->resolveTag(tag);
    
        if (subpatchResult_3)
            return subpatchResult_3;
    
        return nullptr;
    }
    
    DataRef* getDataRef(DataRefIndex index)  {
        switch (index) {
        default:
            {
            return nullptr;
            }
        }
    }
    
    DataRefIndex getNumDataRefs() const {
        return 0;
    }
    
    void processDataViewUpdate(DataRefIndex index, MillisecondTime time) {
        this->updateTime(time, (ENGINE*)nullptr);
    
        if (index == 1) {
            this->mtof_tilde_01_innerMtoF_buffer = reInitDataView(
                this->mtof_tilde_01_innerMtoF_buffer,
                this->getPatcher()->RNBODefaultMtofLookupTable256
            );
        }
    
        this->p_05->processDataViewUpdate(index, time);
        this->p_06->processDataViewUpdate(index, time);
        this->p_07->processDataViewUpdate(index, time);
        this->p_08->processDataViewUpdate(index, time);
    }
    
    void initialize() {
        RNBO_ASSERT(!this->_isInitialized);
        this->assign_defaults();
        this->applyState();
        this->mtof_tilde_01_innerMtoF_buffer = new SampleBuffer(this->getPatcher()->RNBODefaultMtofLookupTable256);
        this->_isInitialized = true;
    }
    
    protected:
    
    class RNBOSubpatcher_949 : public PatcherInterfaceImpl {
            
            friend class RNBOSubpatcher_953;
            friend class rnbomatic;
            
            public:
            
            RNBOSubpatcher_949()
            {}
            
            ~RNBOSubpatcher_949()
            {
                deallocateSignals();
            }
            
            Index getNumMidiInputPorts() const {
                return 1;
            }
            
            void processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->ctlin_01_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
                this->ctlin_02_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
                this->ctlin_03_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
                this->ctlin_04_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
            }
            
            Index getNumMidiOutputPorts() const {
                return 0;
            }
            
            void process(
                const SampleValue * const* inputs,
                Index numInputs,
                SampleValue * const* outputs,
                Index numOutputs,
                Index n
            ) {
                this->vs = n;
                this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
                SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
                SampleValue * out2 = (numOutputs >= 2 && outputs[1] ? outputs[1] : this->dummyBuffer);
                const SampleValue * in1 = (numInputs >= 1 && inputs[0] ? inputs[0] : this->zeroBuffer);
                const SampleValue * in2 = (numInputs >= 2 && inputs[1] ? inputs[1] : this->zeroBuffer);
            
                this->gen_01_perform(
                    in2,
                    in2,
                    this->gen_01_in3,
                    this->gen_01_in4,
                    this->gen_01_in5,
                    this->gen_01_in6,
                    this->gen_01_attack_lockout,
                    this->signals[0],
                    n
                );
            
                this->dspexpr_01_perform(in1, this->signals[0], out1, n);
                this->signalforwarder_01_perform(this->signals[0], out2, n);
                this->stackprotect_perform(n);
                this->audioProcessSampleCount += this->vs;
            }
            
            void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
                RNBO_ASSERT(this->_isInitialized);
            
                if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
                    Index i;
            
                    for (i = 0; i < 1; i++) {
                        this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
                    }
            
                    this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
                    this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
                    this->didAllocateSignals = true;
                }
            
                const bool sampleRateChanged = sampleRate != this->sr;
                const bool maxvsChanged = maxBlockSize != this->maxvs;
                const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;
            
                if (sampleRateChanged || maxvsChanged) {
                    this->vs = maxBlockSize;
                    this->maxvs = maxBlockSize;
                    this->sr = sampleRate;
                    this->invsr = 1 / sampleRate;
                }
            
                this->gen_01_dspsetup(forceDSPSetup);
            
                if (sampleRateChanged)
                    this->onSampleRateChanged(sampleRate);
            }
            
            number msToSamps(MillisecondTime ms, number sampleRate) {
                return ms * sampleRate * 0.001;
            }
            
            MillisecondTime sampsToMs(SampleIndex samps) {
                return samps * (this->invsr * 1000);
            }
            
            Index getNumInputChannels() const {
                return 2;
            }
            
            Index getNumOutputChannels() const {
                return 2;
            }
            
            void getPreset(PatcherStateInterface& preset) {
                this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
                this->param_01_getPresetValue(getSubState(preset, "attack"));
                this->param_02_getPresetValue(getSubState(preset, "decay"));
                this->param_03_getPresetValue(getSubState(preset, "sustain"));
                this->param_04_getPresetValue(getSubState(preset, "release"));
            }
            
            void setPreset(MillisecondTime , PatcherStateInterface& ) {}
            
            void setParameterValue(ParameterIndex index, ParameterValue v, MillisecondTime time) {
                this->updateTime(time, (ENGINE*)nullptr);
            
                switch (index) {
                case 0:
                    {
                    this->param_01_value_set(v);
                    break;
                    }
                case 1:
                    {
                    this->param_02_value_set(v);
                    break;
                    }
                case 2:
                    {
                    this->param_03_value_set(v);
                    break;
                    }
                case 3:
                    {
                    this->param_04_value_set(v);
                    break;
                    }
                }
            }
            
            void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                this->setParameterValue(index, value, time);
            }
            
            void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
                this->setParameterValue(index, this->getParameterValue(index), time);
            }
            
            void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                this->setParameterValueNormalized(index, value, time);
            }
            
            ParameterValue getParameterValue(ParameterIndex index)  {
                switch (index) {
                case 0:
                    {
                    return this->param_01_value;
                    }
                case 1:
                    {
                    return this->param_02_value;
                    }
                case 2:
                    {
                    return this->param_03_value;
                    }
                case 3:
                    {
                    return this->param_04_value;
                    }
                default:
                    {
                    return 0;
                    }
                }
            }
            
            ParameterIndex getNumSignalInParameters() const {
                return 0;
            }
            
            ParameterIndex getNumSignalOutParameters() const {
                return 0;
            }
            
            ParameterIndex getNumParameters() const {
                return 4;
            }
            
            ConstCharPointer getParameterName(ParameterIndex index) const {
                switch (index) {
                case 0:
                    {
                    return "attack";
                    }
                case 1:
                    {
                    return "decay";
                    }
                case 2:
                    {
                    return "sustain";
                    }
                case 3:
                    {
                    return "release";
                    }
                default:
                    {
                    return "bogus";
                    }
                }
            }
            
            ConstCharPointer getParameterId(ParameterIndex index) const {
                switch (index) {
                case 0:
                    {
                    return "poly/p_obj-18/attack";
                    }
                case 1:
                    {
                    return "poly/p_obj-18/decay";
                    }
                case 2:
                    {
                    return "poly/p_obj-18/sustain";
                    }
                case 3:
                    {
                    return "poly/p_obj-18/release";
                    }
                default:
                    {
                    return "bogus";
                    }
                }
            }
            
            void getParameterInfo(ParameterIndex index, ParameterInfo * info) const {
                {
                    switch (index) {
                    case 0:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 30;
                        info->min = 0;
                        info->max = 5000;
                        info->exponent = 3;
                        info->steps = 0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    case 1:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 200;
                        info->min = 1;
                        info->max = 5000;
                        info->exponent = 3;
                        info->steps = 0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    case 2:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 0.5;
                        info->min = 0;
                        info->max = 1;
                        info->exponent = 0.8;
                        info->steps = 0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    case 3:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 300;
                        info->min = 1;
                        info->max = 90000;
                        info->exponent = 5;
                        info->steps = 0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    }
                }
            }
            
            ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
                if (steps == 1) {
                    if (normalizedValue > 0) {
                        normalizedValue = 1.;
                    }
                } else {
                    ParameterValue oneStep = (number)1. / (steps - 1);
                    ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
                    normalizedValue = numberOfSteps * oneStep;
                }
            
                return normalizedValue;
            }
            
            ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                switch (index) {
                case 2:
                    {
                    {
                        value = (value < 0 ? 0 : (value > 1 ? 1 : value));
                        ParameterValue normalizedValue = (value - 0) / (1 - 0);
            
                        {
                            if (normalizedValue != 0.0) {
                                normalizedValue = rnbo_exp(
                                    rnbo_log((normalizedValue <= 0.0000000001 ? 0.0000000001 : normalizedValue)) * 1. / (number)0.8
                                );
                            }
                        }
            
                        return normalizedValue;
                    }
                    }
                case 0:
                    {
                    {
                        value = (value < 0 ? 0 : (value > 5000 ? 5000 : value));
                        ParameterValue normalizedValue = (value - 0) / (5000 - 0);
            
                        {
                            if (normalizedValue != 0.0) {
                                normalizedValue = rnbo_exp(
                                    rnbo_log((normalizedValue <= 0.0000000001 ? 0.0000000001 : normalizedValue)) * 1. / (number)3
                                );
                            }
                        }
            
                        return normalizedValue;
                    }
                    }
                case 1:
                    {
                    {
                        value = (value < 1 ? 1 : (value > 5000 ? 5000 : value));
                        ParameterValue normalizedValue = (value - 1) / (5000 - 1);
            
                        {
                            if (normalizedValue != 0.0) {
                                normalizedValue = rnbo_exp(
                                    rnbo_log((normalizedValue <= 0.0000000001 ? 0.0000000001 : normalizedValue)) * 1. / (number)3
                                );
                            }
                        }
            
                        return normalizedValue;
                    }
                    }
                case 3:
                    {
                    {
                        value = (value < 1 ? 1 : (value > 90000 ? 90000 : value));
                        ParameterValue normalizedValue = (value - 1) / (90000 - 1);
            
                        {
                            if (normalizedValue != 0.0) {
                                normalizedValue = rnbo_exp(
                                    rnbo_log((normalizedValue <= 0.0000000001 ? 0.0000000001 : normalizedValue)) * 1. / (number)5
                                );
                            }
                        }
            
                        return normalizedValue;
                    }
                    }
                default:
                    {
                    return value;
                    }
                }
            }
            
            ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                value = (value < 0 ? 0 : (value > 1 ? 1 : value));
            
                switch (index) {
                case 2:
                    {
                    {
                        {
                            if (value == 0.0) {
                                return 0;
                            } else {
                                return 0 + rnbo_exp(rnbo_log((value <= 0.0000000001 ? 0.0000000001 : value)) * 0.8) * (1 - 0);
                            }
                        }
                    }
                    }
                case 0:
                    {
                    {
                        {
                            if (value == 0.0) {
                                return 0;
                            } else {
                                return 0 + rnbo_exp(rnbo_log((value <= 0.0000000001 ? 0.0000000001 : value)) * 3) * (5000 - 0);
                            }
                        }
                    }
                    }
                case 1:
                    {
                    {
                        {
                            if (value == 0.0) {
                                return 1;
                            } else {
                                return 1 + rnbo_exp(rnbo_log((value <= 0.0000000001 ? 0.0000000001 : value)) * 3) * (5000 - 1);
                            }
                        }
                    }
                    }
                case 3:
                    {
                    {
                        {
                            if (value == 0.0) {
                                return 1;
                            } else {
                                return 1 + rnbo_exp(rnbo_log((value <= 0.0000000001 ? 0.0000000001 : value)) * 5) * (90000 - 1);
                            }
                        }
                    }
                    }
                default:
                    {
                    return value;
                    }
                }
            }
            
            ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
                switch (index) {
                case 0:
                    {
                    return this->param_01_value_constrain(value);
                    }
                case 1:
                    {
                    return this->param_02_value_constrain(value);
                    }
                case 2:
                    {
                    return this->param_03_value_constrain(value);
                    }
                case 3:
                    {
                    return this->param_04_value_constrain(value);
                    }
                default:
                    {
                    return value;
                    }
                }
            }
            
            void processNumMessage(MessageTag , MessageTag , MillisecondTime , number ) {}
            
            void processListMessage(MessageTag , MessageTag , MillisecondTime , const list& ) {}
            
            void processBangMessage(MessageTag , MessageTag , MillisecondTime ) {}
            
            MessageTagInfo resolveTag(MessageTag tag) const {
                switch (tag) {
            
                }
            
                return nullptr;
            }
            
            DataRef* getDataRef(DataRefIndex index)  {
                switch (index) {
                default:
                    {
                    return nullptr;
                    }
                }
            }
            
            DataRefIndex getNumDataRefs() const {
                return 0;
            }
            
            void processDataViewUpdate(DataRefIndex , MillisecondTime ) {}
            
            void initialize() {
                RNBO_ASSERT(!this->_isInitialized);
                this->assign_defaults();
                this->applyState();
                this->_isInitialized = true;
            }
            
            protected:
            
            void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
            	if (time == TimeNow) time = getTopLevelPatcher()->getPatcherTime();
            	getTopLevelPatcher()->processInternalEvents(time);
            	updateTime(time, (EXTERNALENGINE*)nullptr);
            }
            
            RNBOSubpatcher_949* operator->() {
                return this;
            }
            const RNBOSubpatcher_949* operator->() const {
                return this;
            }
            virtual RNBOSubpatcher_953* getPatcher() const {
                return static_cast<RNBOSubpatcher_953 *>(_parentPatcher);
            }
            
            rnbomatic* getTopLevelPatcher() {
                return this->getPatcher()->getTopLevelPatcher();
            }
            
            void cancelClockEvents()
            {
            }
            
            inline number safediv(number num, number denom) {
                return (denom == 0.0 ? 0.0 : num / denom);
            }
            
            number maximum(number x, number y) {
                return (x < y ? y : x);
            }
            
            number mstosamps(MillisecondTime ms) {
                return ms * this->sr * 0.001;
            }
            
            number fromnormalized(Index index, number normalizedValue) {
                return this->convertFromNormalizedParameterValue(index, normalizedValue);
            }
            
            void param_01_value_set(number v) {
                v = this->param_01_value_constrain(v);
                this->param_01_value = v;
                this->sendParameter(0, false);
            
                if (this->param_01_value != this->param_01_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_01_lastValue = this->param_01_value;
                }
            
                this->gen_01_in3_set(v);
            }
            
            void param_02_value_set(number v) {
                v = this->param_02_value_constrain(v);
                this->param_02_value = v;
                this->sendParameter(1, false);
            
                if (this->param_02_value != this->param_02_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_02_lastValue = this->param_02_value;
                }
            
                this->gen_01_in4_set(v);
            }
            
            void param_03_value_set(number v) {
                v = this->param_03_value_constrain(v);
                this->param_03_value = v;
                this->sendParameter(2, false);
            
                if (this->param_03_value != this->param_03_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_03_lastValue = this->param_03_value;
                }
            
                this->gen_01_in5_set(v);
            }
            
            void param_04_value_set(number v) {
                v = this->param_04_value_constrain(v);
                this->param_04_value = v;
                this->sendParameter(3, false);
            
                if (this->param_04_value != this->param_04_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_04_lastValue = this->param_04_value;
                }
            
                this->gen_01_in6_set(v);
            }
            
            MillisecondTime getPatcherTime() const {
                return this->_currentTime;
            }
            
            void deallocateSignals() {
                Index i;
            
                for (i = 0; i < 1; i++) {
                    this->signals[i] = freeSignal(this->signals[i]);
                }
            
                this->zeroBuffer = freeSignal(this->zeroBuffer);
                this->dummyBuffer = freeSignal(this->dummyBuffer);
            }
            
            Index getMaxBlockSize() const {
                return this->maxvs;
            }
            
            number getSampleRate() const {
                return this->sr;
            }
            
            bool hasFixedVectorSize() const {
                return false;
            }
            
            void setProbingTarget(MessageTag ) {}
            
            void initializeObjects() {
                this->gen_01_attack_has_ended_init();
                this->gen_01_sustain_init();
            }
            
            void setVoiceIndex(Index index)  {
                this->_voiceIndex = index;
            }
            
            void setNoteNumber(Int noteNumber)  {
                this->_noteNumber = noteNumber;
            }
            
            Index getIsMuted()  {
                return this->isMuted;
            }
            
            void setIsMuted(Index v)  {
                this->isMuted = v;
            }
            
            void onSampleRateChanged(double ) {}
            
            void extractState(PatcherStateInterface& ) {}
            
            void applyState() {}
            
            void setParameterOffset(ParameterIndex offset) {
                this->parameterOffset = offset;
            }
            
            void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}
            
            void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}
            
            void processOutletEvent(
                EngineLink* sender,
                OutletIndex index,
                ParameterValue value,
                MillisecondTime time
            ) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->processOutletAtCurrentTime(sender, index, value);
            }
            
            void sendOutlet(OutletIndex index, ParameterValue value) {
                this->getEngine()->sendOutlet(this, index, value);
            }
            
            void startup() {
                this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
            
                {
                    this->scheduleParamInit(0, 0);
                }
            
                {
                    this->scheduleParamInit(1, 0);
                }
            
                {
                    this->scheduleParamInit(2, 0);
                }
            
                {
                    this->scheduleParamInit(3, 0);
                }
            }
            
            void fillDataRef(DataRefIndex , DataRef& ) {}
            
            void allocateDataRefs() {}
            
            number param_01_value_constrain(number v) const {
                v = (v > 5000 ? 5000 : (v < 0 ? 0 : v));
                return v;
            }
            
            void gen_01_in3_set(number v) {
                this->gen_01_in3 = v;
            }
            
            number param_02_value_constrain(number v) const {
                v = (v > 5000 ? 5000 : (v < 1 ? 1 : v));
                return v;
            }
            
            void gen_01_in4_set(number v) {
                this->gen_01_in4 = v;
            }
            
            number param_03_value_constrain(number v) const {
                v = (v > 1 ? 1 : (v < 0 ? 0 : v));
                return v;
            }
            
            void gen_01_in5_set(number v) {
                this->gen_01_in5 = v;
            }
            
            number param_04_value_constrain(number v) const {
                v = (v > 90000 ? 90000 : (v < 1 ? 1 : v));
                return v;
            }
            
            void gen_01_in6_set(number v) {
                this->gen_01_in6 = v;
            }
            
            void ctlin_01_outchannel_set(number ) {}
            
            void ctlin_01_outcontroller_set(number ) {}
            
            void fromnormalized_01_output_set(number v) {
                this->param_01_value_set(v);
            }
            
            void fromnormalized_01_input_set(number v) {
                this->fromnormalized_01_output_set(this->fromnormalized(0, v));
            }
            
            void expr_01_out1_set(number v) {
                this->expr_01_out1 = v;
                this->fromnormalized_01_input_set(this->expr_01_out1);
            }
            
            void expr_01_in1_set(number in1) {
                this->expr_01_in1 = in1;
                this->expr_01_out1_set(this->expr_01_in1 * this->expr_01_in2);//#map:expr_01:1
            }
            
            void ctlin_01_value_set(number v) {
                this->expr_01_in1_set(v);
            }
            
            void ctlin_01_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_01_channel || this->ctlin_01_channel == -1) && (data[1] == this->ctlin_01_controller || this->ctlin_01_controller == -1)) {
                    this->ctlin_01_outchannel_set(channel);
                    this->ctlin_01_outcontroller_set(data[1]);
                    this->ctlin_01_value_set(data[2]);
                    this->ctlin_01_status = 0;
                }
            }
            
            void ctlin_02_outchannel_set(number ) {}
            
            void ctlin_02_outcontroller_set(number ) {}
            
            void fromnormalized_02_output_set(number v) {
                this->param_02_value_set(v);
            }
            
            void fromnormalized_02_input_set(number v) {
                this->fromnormalized_02_output_set(this->fromnormalized(1, v));
            }
            
            void expr_02_out1_set(number v) {
                this->expr_02_out1 = v;
                this->fromnormalized_02_input_set(this->expr_02_out1);
            }
            
            void expr_02_in1_set(number in1) {
                this->expr_02_in1 = in1;
                this->expr_02_out1_set(this->expr_02_in1 * this->expr_02_in2);//#map:expr_02:1
            }
            
            void ctlin_02_value_set(number v) {
                this->expr_02_in1_set(v);
            }
            
            void ctlin_02_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_02_channel || this->ctlin_02_channel == -1) && (data[1] == this->ctlin_02_controller || this->ctlin_02_controller == -1)) {
                    this->ctlin_02_outchannel_set(channel);
                    this->ctlin_02_outcontroller_set(data[1]);
                    this->ctlin_02_value_set(data[2]);
                    this->ctlin_02_status = 0;
                }
            }
            
            void ctlin_03_outchannel_set(number ) {}
            
            void ctlin_03_outcontroller_set(number ) {}
            
            void fromnormalized_03_output_set(number v) {
                this->param_03_value_set(v);
            }
            
            void fromnormalized_03_input_set(number v) {
                this->fromnormalized_03_output_set(this->fromnormalized(2, v));
            }
            
            void expr_03_out1_set(number v) {
                this->expr_03_out1 = v;
                this->fromnormalized_03_input_set(this->expr_03_out1);
            }
            
            void expr_03_in1_set(number in1) {
                this->expr_03_in1 = in1;
                this->expr_03_out1_set(this->expr_03_in1 * this->expr_03_in2);//#map:expr_03:1
            }
            
            void ctlin_03_value_set(number v) {
                this->expr_03_in1_set(v);
            }
            
            void ctlin_03_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_03_channel || this->ctlin_03_channel == -1) && (data[1] == this->ctlin_03_controller || this->ctlin_03_controller == -1)) {
                    this->ctlin_03_outchannel_set(channel);
                    this->ctlin_03_outcontroller_set(data[1]);
                    this->ctlin_03_value_set(data[2]);
                    this->ctlin_03_status = 0;
                }
            }
            
            void ctlin_04_outchannel_set(number ) {}
            
            void ctlin_04_outcontroller_set(number ) {}
            
            void fromnormalized_04_output_set(number v) {
                this->param_04_value_set(v);
            }
            
            void fromnormalized_04_input_set(number v) {
                this->fromnormalized_04_output_set(this->fromnormalized(3, v));
            }
            
            void expr_04_out1_set(number v) {
                this->expr_04_out1 = v;
                this->fromnormalized_04_input_set(this->expr_04_out1);
            }
            
            void expr_04_in1_set(number in1) {
                this->expr_04_in1 = in1;
                this->expr_04_out1_set(this->expr_04_in1 * this->expr_04_in2);//#map:expr_04:1
            }
            
            void ctlin_04_value_set(number v) {
                this->expr_04_in1_set(v);
            }
            
            void ctlin_04_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_04_channel || this->ctlin_04_channel == -1) && (data[1] == this->ctlin_04_controller || this->ctlin_04_controller == -1)) {
                    this->ctlin_04_outchannel_set(channel);
                    this->ctlin_04_outcontroller_set(data[1]);
                    this->ctlin_04_value_set(data[2]);
                    this->ctlin_04_status = 0;
                }
            }
            
            void gen_01_perform(
                const Sample * in1,
                const Sample * in2,
                number in3,
                number in4,
                number in5,
                number in6,
                number attack_lockout,
                SampleValue * out1,
                Index n
            ) {
                RNBO_UNUSED(attack_lockout);
                auto __gen_01_sustain_value = this->gen_01_sustain_value;
                auto __gen_01_attack_has_ended_value = this->gen_01_attack_has_ended_value;
                auto min_1_0 = this->maximum(in3, 0.1);
                auto min_2_1 = this->maximum(in4, 0.1);
                number clamp_3_2 = (in5 > 1 ? 1 : (in5 < 0 ? 0 : in5));
                auto min_4_3 = this->maximum(in6, 0.1);
                number attack_iter_interval_8 = this->mstosamps(min_1_0) * 0.33333333333333;
                number release_iter_interval_9 = this->mstosamps(min_4_3) * 0.33333333333333;
                number decay_iter_interval_10 = this->mstosamps(min_2_1) * 0.33333333333333;
                number sustain_smoothing_12 = 1 - ((this->mstosamps(20) == 0. ? 0. : (number)1 / this->mstosamps(20)));
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    number trigger_5 = this->gen_01_delta_4_next(in2[(Index)i] > 0) > 0;
                    number is_released_6 = in1[(Index)i] == 0;
                    number is_attacking_7 = ((bool)(trigger_5) || (bool)(!(bool)(__gen_01_attack_has_ended_value))) && ((bool)(!(bool)(is_released_6)) || 0);
                    number decay_iter_interval_5_11 = !(bool)(is_released_6) * decay_iter_interval_10 + is_released_6 * release_iter_interval_9;
                    __gen_01_sustain_value = clamp_3_2 + sustain_smoothing_12 * (__gen_01_sustain_value - clamp_3_2);
                    number ad_target_13 = (is_attacking_7 > 0 ? 1.0523956964913 : -0.052395696491256);
                    number asr_target_14 = ((bool)(!(bool)(is_released_6)) || (bool)(is_attacking_7) ? 1.0523956964913 : -0.052395696491256);
                    number ad_16 = this->gen_01_slide_15_next(ad_target_13, attack_iter_interval_8, decay_iter_interval_5_11);
                    number ad_6_17 = (ad_16 > 1 ? 1 : (ad_16 < 0 ? 0 : ad_16));
                    number asr_19 = this->gen_01_slide_18_next(asr_target_14, attack_iter_interval_8, release_iter_interval_9);
                    number asr_7_20 = (asr_19 > 1 ? 1 : (asr_19 < 0 ? 0 : asr_19));
                    __gen_01_attack_has_ended_value = (bool)(!(bool)(is_attacking_7)) || ad_6_17 >= 1;
                    number expr_8_21 = ad_6_17 + __gen_01_sustain_value * (asr_7_20 - ad_6_17);
                    out1[(Index)i] = expr_8_21;
                }
            
                this->gen_01_attack_has_ended_value = __gen_01_attack_has_ended_value;
                this->gen_01_sustain_value = __gen_01_sustain_value;
            }
            
            void dspexpr_01_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
                }
            }
            
            void signalforwarder_01_perform(const SampleValue * input, SampleValue * output, Index n) {
                copySignal(output, input, n);
            }
            
            void stackprotect_perform(Index n) {
                RNBO_UNUSED(n);
                auto __stackprotect_count = this->stackprotect_count;
                __stackprotect_count = 0;
                this->stackprotect_count = __stackprotect_count;
            }
            
            number gen_01_attack_has_ended_getvalue() {
                return this->gen_01_attack_has_ended_value;
            }
            
            void gen_01_attack_has_ended_setvalue(number val) {
                this->gen_01_attack_has_ended_value = val;
            }
            
            void gen_01_attack_has_ended_reset() {
                this->gen_01_attack_has_ended_value = 0;
            }
            
            void gen_01_attack_has_ended_init() {
                this->gen_01_attack_has_ended_value = 0;
            }
            
            number gen_01_sustain_getvalue() {
                return this->gen_01_sustain_value;
            }
            
            void gen_01_sustain_setvalue(number val) {
                this->gen_01_sustain_value = val;
            }
            
            void gen_01_sustain_reset() {
                this->gen_01_sustain_value = 0;
            }
            
            void gen_01_sustain_init() {
                this->gen_01_sustain_value = 0.5;
            }
            
            number gen_01_delta_4_next(number x) {
                number temp = (number)(x - this->gen_01_delta_4_prev);
                this->gen_01_delta_4_prev = x;
                return temp;
            }
            
            void gen_01_delta_4_dspsetup() {
                this->gen_01_delta_4_reset();
            }
            
            void gen_01_delta_4_reset() {
                this->gen_01_delta_4_prev = 0;
            }
            
            number gen_01_slide_15_next(number x, number up, number down) {
                number temp = x - this->gen_01_slide_15_prev;
                auto iup = this->safediv(1., this->maximum(1., rnbo_abs(up)));
                auto idown = this->safediv(1., this->maximum(1., rnbo_abs(down)));
                this->gen_01_slide_15_prev = this->gen_01_slide_15_prev + ((x > this->gen_01_slide_15_prev ? iup : idown)) * temp;
                return this->gen_01_slide_15_prev;
            }
            
            void gen_01_slide_15_reset() {
                this->gen_01_slide_15_prev = 0;
            }
            
            number gen_01_slide_18_next(number x, number up, number down) {
                number temp = x - this->gen_01_slide_18_prev;
                auto iup = this->safediv(1., this->maximum(1., rnbo_abs(up)));
                auto idown = this->safediv(1., this->maximum(1., rnbo_abs(down)));
                this->gen_01_slide_18_prev = this->gen_01_slide_18_prev + ((x > this->gen_01_slide_18_prev ? iup : idown)) * temp;
                return this->gen_01_slide_18_prev;
            }
            
            void gen_01_slide_18_reset() {
                this->gen_01_slide_18_prev = 0;
            }
            
            void gen_01_dspsetup(bool force) {
                if ((bool)(this->gen_01_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->gen_01_setupDone = true;
                this->gen_01_delta_4_dspsetup();
            }
            
            void param_01_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_01_value;
            }
            
            void param_01_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_01_value_set(preset["value"]);
            }
            
            void param_02_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_02_value;
            }
            
            void param_02_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_02_value_set(preset["value"]);
            }
            
            void param_03_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_03_value;
            }
            
            void param_03_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_03_value_set(preset["value"]);
            }
            
            void param_04_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_04_value;
            }
            
            void param_04_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_04_value_set(preset["value"]);
            }
            
            bool stackprotect_check() {
                this->stackprotect_count++;
            
                if (this->stackprotect_count > 128) {
                    console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
                    return true;
                }
            
                return false;
            }
            
            Index getPatcherSerial() const {
                return 0;
            }
            
            void sendParameter(ParameterIndex index, bool ignoreValue) {
                this->getPatcher()->sendParameter(index + this->parameterOffset, ignoreValue);
            }
            
            void scheduleParamInit(ParameterIndex index, Index order) {
                this->getPatcher()->scheduleParamInit(index + this->parameterOffset, order);
            }
            
            void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
                RNBO_UNUSED(inProcess);
                RNBO_UNUSED(engine);
                this->_currentTime = time;
                auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));
            
                if (offset >= (SampleIndex)(this->vs))
                    offset = (SampleIndex)(this->vs) - 1;
            
                if (offset < 0)
                    offset = 0;
            
                this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
            }
            
            void assign_defaults()
            {
                dspexpr_01_in1 = 0;
                dspexpr_01_in2 = 0;
                gen_01_in1 = 0;
                gen_01_in2 = 0;
                gen_01_in3 = 0;
                gen_01_in4 = 0;
                gen_01_in5 = 0;
                gen_01_in6 = 0;
                gen_01_attack_lockout = 0;
                param_01_value = 30;
                param_02_value = 200;
                param_03_value = 0.5;
                param_04_value = 300;
                ctlin_01_input = 0;
                ctlin_01_controller = 0;
                ctlin_01_channel = -1;
                expr_01_in1 = 0;
                expr_01_in2 = 0.007874015748;
                expr_01_out1 = 0;
                ctlin_02_input = 0;
                ctlin_02_controller = 0;
                ctlin_02_channel = -1;
                expr_02_in1 = 0;
                expr_02_in2 = 0.007874015748;
                expr_02_out1 = 0;
                ctlin_03_input = 0;
                ctlin_03_controller = 0;
                ctlin_03_channel = -1;
                expr_03_in1 = 0;
                expr_03_in2 = 0.007874015748;
                expr_03_out1 = 0;
                ctlin_04_input = 0;
                ctlin_04_controller = 0;
                ctlin_04_channel = -1;
                expr_04_in1 = 0;
                expr_04_in2 = 0.007874015748;
                expr_04_out1 = 0;
                _currentTime = 0;
                audioProcessSampleCount = 0;
                sampleOffsetIntoNextAudioBuffer = 0;
                zeroBuffer = nullptr;
                dummyBuffer = nullptr;
                signals[0] = nullptr;
                didAllocateSignals = 0;
                vs = 0;
                maxvs = 0;
                sr = 44100;
                invsr = 0.000022675736961451248;
                gen_01_attack_has_ended_value = 0;
                gen_01_sustain_value = 0;
                gen_01_delta_4_prev = 0;
                gen_01_slide_15_prev = 0;
                gen_01_slide_18_prev = 0;
                gen_01_setupDone = false;
                param_01_lastValue = 0;
                param_02_lastValue = 0;
                param_03_lastValue = 0;
                param_04_lastValue = 0;
                ctlin_01_status = 0;
                ctlin_01_byte1 = -1;
                ctlin_01_inchan = 0;
                ctlin_02_status = 0;
                ctlin_02_byte1 = -1;
                ctlin_02_inchan = 0;
                ctlin_03_status = 0;
                ctlin_03_byte1 = -1;
                ctlin_03_inchan = 0;
                ctlin_04_status = 0;
                ctlin_04_byte1 = -1;
                ctlin_04_inchan = 0;
                stackprotect_count = 0;
                _voiceIndex = 0;
                _noteNumber = 0;
                isMuted = 1;
                parameterOffset = 0;
            }
            
            // member variables
            
                number dspexpr_01_in1;
                number dspexpr_01_in2;
                number gen_01_in1;
                number gen_01_in2;
                number gen_01_in3;
                number gen_01_in4;
                number gen_01_in5;
                number gen_01_in6;
                number gen_01_attack_lockout;
                number param_01_value;
                number param_02_value;
                number param_03_value;
                number param_04_value;
                number ctlin_01_input;
                number ctlin_01_controller;
                number ctlin_01_channel;
                number expr_01_in1;
                number expr_01_in2;
                number expr_01_out1;
                number ctlin_02_input;
                number ctlin_02_controller;
                number ctlin_02_channel;
                number expr_02_in1;
                number expr_02_in2;
                number expr_02_out1;
                number ctlin_03_input;
                number ctlin_03_controller;
                number ctlin_03_channel;
                number expr_03_in1;
                number expr_03_in2;
                number expr_03_out1;
                number ctlin_04_input;
                number ctlin_04_controller;
                number ctlin_04_channel;
                number expr_04_in1;
                number expr_04_in2;
                number expr_04_out1;
                MillisecondTime _currentTime;
                UInt64 audioProcessSampleCount;
                Index sampleOffsetIntoNextAudioBuffer;
                signal zeroBuffer;
                signal dummyBuffer;
                SampleValue * signals[1];
                bool didAllocateSignals;
                Index vs;
                Index maxvs;
                number sr;
                number invsr;
                number gen_01_attack_has_ended_value;
                number gen_01_sustain_value;
                number gen_01_delta_4_prev;
                number gen_01_slide_15_prev;
                number gen_01_slide_18_prev;
                bool gen_01_setupDone;
                number param_01_lastValue;
                number param_02_lastValue;
                number param_03_lastValue;
                number param_04_lastValue;
                Int ctlin_01_status;
                Int ctlin_01_byte1;
                Int ctlin_01_inchan;
                Int ctlin_02_status;
                Int ctlin_02_byte1;
                Int ctlin_02_inchan;
                Int ctlin_03_status;
                Int ctlin_03_byte1;
                Int ctlin_03_inchan;
                Int ctlin_04_status;
                Int ctlin_04_byte1;
                Int ctlin_04_inchan;
                number stackprotect_count;
                Index _voiceIndex;
                Int _noteNumber;
                Index isMuted;
                ParameterIndex parameterOffset;
                bool _isInitialized = false;
    };
    
    class RNBOSubpatcher_950 : public PatcherInterfaceImpl {
            
            friend class RNBOSubpatcher_953;
            friend class rnbomatic;
            
            public:
            
            RNBOSubpatcher_950()
            {}
            
            ~RNBOSubpatcher_950()
            {
                deallocateSignals();
            }
            
            Index getNumMidiInputPorts() const {
                return 0;
            }
            
            void processMidiEvent(MillisecondTime , int , ConstByteArray , Index ) {}
            
            Index getNumMidiOutputPorts() const {
                return 0;
            }
            
            void process(
                const SampleValue * const* inputs,
                Index numInputs,
                SampleValue * const* outputs,
                Index numOutputs,
                Index n
            ) {
                this->vs = n;
                this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
                SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
                SampleValue * out2 = (numOutputs >= 2 && outputs[1] ? outputs[1] : this->dummyBuffer);
                const SampleValue * in1 = (numInputs >= 1 && inputs[0] ? inputs[0] : this->zeroBuffer);
                const SampleValue * in2 = (numInputs >= 2 && inputs[1] ? inputs[1] : this->zeroBuffer);
                const SampleValue * in3 = (numInputs >= 3 && inputs[2] ? inputs[2] : this->zeroBuffer);
                this->dspexpr_03_perform(in1, in2, this->signals[0], n);
            
                this->cycle_tilde_02_perform(
                    this->signals[0],
                    this->cycle_tilde_02_phase_offset,
                    this->signals[1],
                    this->dummyBuffer,
                    n
                );
            
                this->dspexpr_05_perform(this->signals[0], in3, this->signals[2], n);
                this->dspexpr_04_perform(this->signals[1], this->signals[2], this->signals[0], n);
                this->dspexpr_02_perform(in1, this->signals[0], this->signals[2], n);
                this->cycle_tilde_01_perform(this->signals[2], this->cycle_tilde_01_phase_offset, out1, out2, n);
                this->stackprotect_perform(n);
                this->audioProcessSampleCount += this->vs;
            }
            
            void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
                RNBO_ASSERT(this->_isInitialized);
            
                if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
                    Index i;
            
                    for (i = 0; i < 3; i++) {
                        this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
                    }
            
                    this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
                    this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
                    this->didAllocateSignals = true;
                }
            
                const bool sampleRateChanged = sampleRate != this->sr;
                const bool maxvsChanged = maxBlockSize != this->maxvs;
                const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;
            
                if (sampleRateChanged || maxvsChanged) {
                    this->vs = maxBlockSize;
                    this->maxvs = maxBlockSize;
                    this->sr = sampleRate;
                    this->invsr = 1 / sampleRate;
                }
            
                this->cycle_tilde_02_dspsetup(forceDSPSetup);
                this->cycle_tilde_01_dspsetup(forceDSPSetup);
            
                if (sampleRateChanged)
                    this->onSampleRateChanged(sampleRate);
            }
            
            number msToSamps(MillisecondTime ms, number sampleRate) {
                return ms * sampleRate * 0.001;
            }
            
            MillisecondTime sampsToMs(SampleIndex samps) {
                return samps * (this->invsr * 1000);
            }
            
            Index getNumInputChannels() const {
                return 3;
            }
            
            Index getNumOutputChannels() const {
                return 2;
            }
            
            void getPreset(PatcherStateInterface& ) {}
            
            void setPreset(MillisecondTime , PatcherStateInterface& ) {}
            
            void setParameterValue(ParameterIndex , ParameterValue , MillisecondTime ) {}
            
            void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                this->setParameterValue(index, value, time);
            }
            
            void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
                this->setParameterValue(index, this->getParameterValue(index), time);
            }
            
            void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                this->setParameterValueNormalized(index, value, time);
            }
            
            ParameterValue getParameterValue(ParameterIndex index)  {
                switch (index) {
                default:
                    {
                    return 0;
                    }
                }
            }
            
            ParameterIndex getNumSignalInParameters() const {
                return 0;
            }
            
            ParameterIndex getNumSignalOutParameters() const {
                return 0;
            }
            
            ParameterIndex getNumParameters() const {
                return 0;
            }
            
            ConstCharPointer getParameterName(ParameterIndex index) const {
                switch (index) {
                default:
                    {
                    return "bogus";
                    }
                }
            }
            
            ConstCharPointer getParameterId(ParameterIndex index) const {
                switch (index) {
                default:
                    {
                    return "bogus";
                    }
                }
            }
            
            void getParameterInfo(ParameterIndex , ParameterInfo * ) const {}
            
            ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
                if (steps == 1) {
                    if (normalizedValue > 0) {
                        normalizedValue = 1.;
                    }
                } else {
                    ParameterValue oneStep = (number)1. / (steps - 1);
                    ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
                    normalizedValue = numberOfSteps * oneStep;
                }
            
                return normalizedValue;
            }
            
            ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                switch (index) {
                default:
                    {
                    return value;
                    }
                }
            }
            
            ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                value = (value < 0 ? 0 : (value > 1 ? 1 : value));
            
                switch (index) {
                default:
                    {
                    return value;
                    }
                }
            }
            
            ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
                switch (index) {
                default:
                    {
                    return value;
                    }
                }
            }
            
            void processNumMessage(MessageTag , MessageTag , MillisecondTime , number ) {}
            
            void processListMessage(MessageTag , MessageTag , MillisecondTime , const list& ) {}
            
            void processBangMessage(MessageTag , MessageTag , MillisecondTime ) {}
            
            MessageTagInfo resolveTag(MessageTag tag) const {
                switch (tag) {
            
                }
            
                return nullptr;
            }
            
            DataRef* getDataRef(DataRefIndex index)  {
                switch (index) {
                default:
                    {
                    return nullptr;
                    }
                }
            }
            
            DataRefIndex getNumDataRefs() const {
                return 0;
            }
            
            void processDataViewUpdate(DataRefIndex index, MillisecondTime time) {
                this->updateTime(time, (ENGINE*)nullptr);
            
                if (index == 0) {
                    this->cycle_tilde_01_buffer = reInitDataView(
                        this->cycle_tilde_01_buffer,
                        this->getPatcher()->getPatcher()->RNBODefaultSinus
                    );
            
                    this->cycle_tilde_01_bufferUpdated();
            
                    this->cycle_tilde_02_buffer = reInitDataView(
                        this->cycle_tilde_02_buffer,
                        this->getPatcher()->getPatcher()->RNBODefaultSinus
                    );
            
                    this->cycle_tilde_02_bufferUpdated();
                }
            }
            
            void initialize() {
                RNBO_ASSERT(!this->_isInitialized);
                this->assign_defaults();
                this->applyState();
                this->cycle_tilde_01_buffer = new SampleBuffer(this->getPatcher()->getPatcher()->RNBODefaultSinus);
                this->cycle_tilde_02_buffer = new SampleBuffer(this->getPatcher()->getPatcher()->RNBODefaultSinus);
                this->_isInitialized = true;
            }
            
            protected:
            
            void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
            	if (time == TimeNow) time = getTopLevelPatcher()->getPatcherTime();
            	getTopLevelPatcher()->processInternalEvents(time);
            	updateTime(time, (EXTERNALENGINE*)nullptr);
            }
            
            RNBOSubpatcher_950* operator->() {
                return this;
            }
            const RNBOSubpatcher_950* operator->() const {
                return this;
            }
            virtual RNBOSubpatcher_953* getPatcher() const {
                return static_cast<RNBOSubpatcher_953 *>(_parentPatcher);
            }
            
            rnbomatic* getTopLevelPatcher() {
                return this->getPatcher()->getTopLevelPatcher();
            }
            
            void cancelClockEvents()
            {
            }
            
            MillisecondTime getPatcherTime() const {
                return this->_currentTime;
            }
            
            void deallocateSignals() {
                Index i;
            
                for (i = 0; i < 3; i++) {
                    this->signals[i] = freeSignal(this->signals[i]);
                }
            
                this->zeroBuffer = freeSignal(this->zeroBuffer);
                this->dummyBuffer = freeSignal(this->dummyBuffer);
            }
            
            Index getMaxBlockSize() const {
                return this->maxvs;
            }
            
            number getSampleRate() const {
                return this->sr;
            }
            
            bool hasFixedVectorSize() const {
                return false;
            }
            
            void setProbingTarget(MessageTag ) {}
            
            void initializeObjects() {}
            
            void setVoiceIndex(Index index)  {
                this->_voiceIndex = index;
            }
            
            void setNoteNumber(Int noteNumber)  {
                this->_noteNumber = noteNumber;
            }
            
            Index getIsMuted()  {
                return this->isMuted;
            }
            
            void setIsMuted(Index v)  {
                this->isMuted = v;
            }
            
            void onSampleRateChanged(double ) {}
            
            void extractState(PatcherStateInterface& ) {}
            
            void applyState() {}
            
            void setParameterOffset(ParameterIndex offset) {
                this->parameterOffset = offset;
            }
            
            void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}
            
            void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}
            
            void processOutletEvent(
                EngineLink* sender,
                OutletIndex index,
                ParameterValue value,
                MillisecondTime time
            ) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->processOutletAtCurrentTime(sender, index, value);
            }
            
            void sendOutlet(OutletIndex index, ParameterValue value) {
                this->getEngine()->sendOutlet(this, index, value);
            }
            
            void startup() {}
            
            void fillDataRef(DataRefIndex , DataRef& ) {}
            
            void allocateDataRefs() {
                this->cycle_tilde_01_buffer->requestSize(16384, 1);
                this->cycle_tilde_01_buffer->setSampleRate(this->sr);
                this->cycle_tilde_02_buffer->requestSize(16384, 1);
                this->cycle_tilde_02_buffer->setSampleRate(this->sr);
                this->cycle_tilde_01_buffer = this->cycle_tilde_01_buffer->allocateIfNeeded();
                this->cycle_tilde_02_buffer = this->cycle_tilde_02_buffer->allocateIfNeeded();
            }
            
            void dspexpr_03_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
                }
            }
            
            void cycle_tilde_02_perform(
                const Sample * frequency,
                number phase_offset,
                SampleValue * out1,
                SampleValue * out2,
                Index n
            ) {
                RNBO_UNUSED(phase_offset);
                auto __cycle_tilde_02_f2i = this->cycle_tilde_02_f2i;
                auto __cycle_tilde_02_buffer = this->cycle_tilde_02_buffer;
                auto __cycle_tilde_02_phasei = this->cycle_tilde_02_phasei;
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    {
                        UInt32 uint_phase;
            
                        {
                            {
                                uint_phase = __cycle_tilde_02_phasei;
                            }
                        }
            
                        UInt32 idx = (UInt32)(uint32_rshift(uint_phase, 18));
                        number frac = ((BinOpInt)((BinOpInt)uint_phase & (BinOpInt)262143)) * 3.81471181759574e-6;
                        number y0 = __cycle_tilde_02_buffer[(Index)idx];
                        number y1 = __cycle_tilde_02_buffer[(Index)((BinOpInt)(idx + 1) & (BinOpInt)16383)];
                        number y = y0 + frac * (y1 - y0);
            
                        {
                            UInt32 pincr = (UInt32)(uint32_trunc(frequency[(Index)i] * __cycle_tilde_02_f2i));
                            __cycle_tilde_02_phasei = uint32_add(__cycle_tilde_02_phasei, pincr);
                        }
            
                        out1[(Index)i] = y;
                        out2[(Index)i] = uint_phase * 0.232830643653869629e-9;
                        continue;
                    }
                }
            
                this->cycle_tilde_02_phasei = __cycle_tilde_02_phasei;
            }
            
            void dspexpr_05_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
                }
            }
            
            void dspexpr_04_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
                }
            }
            
            void dspexpr_02_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out1[(Index)i] = in1[(Index)i] + in2[(Index)i];//#map:_###_obj_###_:1
                }
            }
            
            void cycle_tilde_01_perform(
                const Sample * frequency,
                number phase_offset,
                SampleValue * out1,
                SampleValue * out2,
                Index n
            ) {
                RNBO_UNUSED(phase_offset);
                auto __cycle_tilde_01_f2i = this->cycle_tilde_01_f2i;
                auto __cycle_tilde_01_buffer = this->cycle_tilde_01_buffer;
                auto __cycle_tilde_01_phasei = this->cycle_tilde_01_phasei;
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    {
                        UInt32 uint_phase;
            
                        {
                            {
                                uint_phase = __cycle_tilde_01_phasei;
                            }
                        }
            
                        UInt32 idx = (UInt32)(uint32_rshift(uint_phase, 18));
                        number frac = ((BinOpInt)((BinOpInt)uint_phase & (BinOpInt)262143)) * 3.81471181759574e-6;
                        number y0 = __cycle_tilde_01_buffer[(Index)idx];
                        number y1 = __cycle_tilde_01_buffer[(Index)((BinOpInt)(idx + 1) & (BinOpInt)16383)];
                        number y = y0 + frac * (y1 - y0);
            
                        {
                            UInt32 pincr = (UInt32)(uint32_trunc(frequency[(Index)i] * __cycle_tilde_01_f2i));
                            __cycle_tilde_01_phasei = uint32_add(__cycle_tilde_01_phasei, pincr);
                        }
            
                        out1[(Index)i] = y;
                        out2[(Index)i] = uint_phase * 0.232830643653869629e-9;
                        continue;
                    }
                }
            
                this->cycle_tilde_01_phasei = __cycle_tilde_01_phasei;
            }
            
            void stackprotect_perform(Index n) {
                RNBO_UNUSED(n);
                auto __stackprotect_count = this->stackprotect_count;
                __stackprotect_count = 0;
                this->stackprotect_count = __stackprotect_count;
            }
            
            number cycle_tilde_01_ph_next(number freq, number reset) {
                {
                    {
                        if (reset >= 0.)
                            this->cycle_tilde_01_ph_currentPhase = reset;
                    }
                }
            
                number pincr = freq * this->cycle_tilde_01_ph_conv;
            
                if (this->cycle_tilde_01_ph_currentPhase < 0.)
                    this->cycle_tilde_01_ph_currentPhase = 1. + this->cycle_tilde_01_ph_currentPhase;
            
                if (this->cycle_tilde_01_ph_currentPhase > 1.)
                    this->cycle_tilde_01_ph_currentPhase = this->cycle_tilde_01_ph_currentPhase - 1.;
            
                number tmp = this->cycle_tilde_01_ph_currentPhase;
                this->cycle_tilde_01_ph_currentPhase += pincr;
                return tmp;
            }
            
            void cycle_tilde_01_ph_reset() {
                this->cycle_tilde_01_ph_currentPhase = 0;
            }
            
            void cycle_tilde_01_ph_dspsetup() {
                this->cycle_tilde_01_ph_conv = (number)1 / this->sr;
            }
            
            void cycle_tilde_01_dspsetup(bool force) {
                if ((bool)(this->cycle_tilde_01_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->cycle_tilde_01_phasei = 0;
                this->cycle_tilde_01_f2i = (number)4294967296 / this->sr;
                this->cycle_tilde_01_wrap = (Int)(this->cycle_tilde_01_buffer->getSize()) - 1;
                this->cycle_tilde_01_setupDone = true;
                this->cycle_tilde_01_ph_dspsetup();
            }
            
            void cycle_tilde_01_bufferUpdated() {
                this->cycle_tilde_01_wrap = (Int)(this->cycle_tilde_01_buffer->getSize()) - 1;
            }
            
            number cycle_tilde_02_ph_next(number freq, number reset) {
                {
                    {
                        if (reset >= 0.)
                            this->cycle_tilde_02_ph_currentPhase = reset;
                    }
                }
            
                number pincr = freq * this->cycle_tilde_02_ph_conv;
            
                if (this->cycle_tilde_02_ph_currentPhase < 0.)
                    this->cycle_tilde_02_ph_currentPhase = 1. + this->cycle_tilde_02_ph_currentPhase;
            
                if (this->cycle_tilde_02_ph_currentPhase > 1.)
                    this->cycle_tilde_02_ph_currentPhase = this->cycle_tilde_02_ph_currentPhase - 1.;
            
                number tmp = this->cycle_tilde_02_ph_currentPhase;
                this->cycle_tilde_02_ph_currentPhase += pincr;
                return tmp;
            }
            
            void cycle_tilde_02_ph_reset() {
                this->cycle_tilde_02_ph_currentPhase = 0;
            }
            
            void cycle_tilde_02_ph_dspsetup() {
                this->cycle_tilde_02_ph_conv = (number)1 / this->sr;
            }
            
            void cycle_tilde_02_dspsetup(bool force) {
                if ((bool)(this->cycle_tilde_02_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->cycle_tilde_02_phasei = 0;
                this->cycle_tilde_02_f2i = (number)4294967296 / this->sr;
                this->cycle_tilde_02_wrap = (Int)(this->cycle_tilde_02_buffer->getSize()) - 1;
                this->cycle_tilde_02_setupDone = true;
                this->cycle_tilde_02_ph_dspsetup();
            }
            
            void cycle_tilde_02_bufferUpdated() {
                this->cycle_tilde_02_wrap = (Int)(this->cycle_tilde_02_buffer->getSize()) - 1;
            }
            
            bool stackprotect_check() {
                this->stackprotect_count++;
            
                if (this->stackprotect_count > 128) {
                    console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
                    return true;
                }
            
                return false;
            }
            
            Index getPatcherSerial() const {
                return 0;
            }
            
            void sendParameter(ParameterIndex index, bool ignoreValue) {
                this->getPatcher()->sendParameter(index + this->parameterOffset, ignoreValue);
            }
            
            void scheduleParamInit(ParameterIndex index, Index order) {
                this->getPatcher()->scheduleParamInit(index + this->parameterOffset, order);
            }
            
            void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
                RNBO_UNUSED(inProcess);
                RNBO_UNUSED(engine);
                this->_currentTime = time;
                auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));
            
                if (offset >= (SampleIndex)(this->vs))
                    offset = (SampleIndex)(this->vs) - 1;
            
                if (offset < 0)
                    offset = 0;
            
                this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
            }
            
            void assign_defaults()
            {
                cycle_tilde_01_frequency = 0;
                cycle_tilde_01_phase_offset = 0;
                dspexpr_02_in1 = 0;
                dspexpr_02_in2 = 0;
                dspexpr_03_in1 = 0;
                dspexpr_03_in2 = 0;
                cycle_tilde_02_frequency = 0;
                cycle_tilde_02_phase_offset = 0;
                dspexpr_04_in1 = 0;
                dspexpr_04_in2 = 0;
                dspexpr_05_in1 = 0;
                dspexpr_05_in2 = 0;
                _currentTime = 0;
                audioProcessSampleCount = 0;
                sampleOffsetIntoNextAudioBuffer = 0;
                zeroBuffer = nullptr;
                dummyBuffer = nullptr;
                signals[0] = nullptr;
                signals[1] = nullptr;
                signals[2] = nullptr;
                didAllocateSignals = 0;
                vs = 0;
                maxvs = 0;
                sr = 44100;
                invsr = 0.000022675736961451248;
                cycle_tilde_01_wrap = 0;
                cycle_tilde_01_ph_currentPhase = 0;
                cycle_tilde_01_ph_conv = 0;
                cycle_tilde_01_setupDone = false;
                cycle_tilde_02_wrap = 0;
                cycle_tilde_02_ph_currentPhase = 0;
                cycle_tilde_02_ph_conv = 0;
                cycle_tilde_02_setupDone = false;
                stackprotect_count = 0;
                _voiceIndex = 0;
                _noteNumber = 0;
                isMuted = 1;
                parameterOffset = 0;
            }
            
            // member variables
            
                number cycle_tilde_01_frequency;
                number cycle_tilde_01_phase_offset;
                number dspexpr_02_in1;
                number dspexpr_02_in2;
                number dspexpr_03_in1;
                number dspexpr_03_in2;
                number cycle_tilde_02_frequency;
                number cycle_tilde_02_phase_offset;
                number dspexpr_04_in1;
                number dspexpr_04_in2;
                number dspexpr_05_in1;
                number dspexpr_05_in2;
                MillisecondTime _currentTime;
                UInt64 audioProcessSampleCount;
                Index sampleOffsetIntoNextAudioBuffer;
                signal zeroBuffer;
                signal dummyBuffer;
                SampleValue * signals[3];
                bool didAllocateSignals;
                Index vs;
                Index maxvs;
                number sr;
                number invsr;
                SampleBufferRef cycle_tilde_01_buffer;
                Int cycle_tilde_01_wrap;
                UInt32 cycle_tilde_01_phasei;
                SampleValue cycle_tilde_01_f2i;
                number cycle_tilde_01_ph_currentPhase;
                number cycle_tilde_01_ph_conv;
                bool cycle_tilde_01_setupDone;
                SampleBufferRef cycle_tilde_02_buffer;
                Int cycle_tilde_02_wrap;
                UInt32 cycle_tilde_02_phasei;
                SampleValue cycle_tilde_02_f2i;
                number cycle_tilde_02_ph_currentPhase;
                number cycle_tilde_02_ph_conv;
                bool cycle_tilde_02_setupDone;
                number stackprotect_count;
                Index _voiceIndex;
                Int _noteNumber;
                Index isMuted;
                ParameterIndex parameterOffset;
                bool _isInitialized = false;
    };
    
    class RNBOSubpatcher_951 : public PatcherInterfaceImpl {
            
            friend class RNBOSubpatcher_953;
            friend class rnbomatic;
            
            public:
            
            RNBOSubpatcher_951()
            {}
            
            ~RNBOSubpatcher_951()
            {
                deallocateSignals();
            }
            
            Index getNumMidiInputPorts() const {
                return 1;
            }
            
            void processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->ctlin_05_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
                this->ctlin_06_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
                this->ctlin_07_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
            }
            
            Index getNumMidiOutputPorts() const {
                return 0;
            }
            
            void process(
                const SampleValue * const* inputs,
                Index numInputs,
                SampleValue * const* outputs,
                Index numOutputs,
                Index n
            ) {
                this->vs = n;
                this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
                SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
                SampleValue * out2 = (numOutputs >= 2 && outputs[1] ? outputs[1] : this->dummyBuffer);
                const SampleValue * in1 = (numInputs >= 1 && inputs[0] ? inputs[0] : this->zeroBuffer);
                this->dspexpr_07_perform(in1, this->dspexpr_07_in2, this->signals[0], n);
                this->dspexpr_09_perform(in1, this->dspexpr_09_in2, this->signals[1], n);
            
                this->slide_tilde_01_perform(
                    this->slide_tilde_01_x,
                    this->slide_tilde_01_up,
                    this->slide_tilde_01_down,
                    this->signals[2],
                    n
                );
            
                this->feedbackreader_01_perform(this->signals[3], n);
                this->delaytilde_01_perform(this->delaytilde_01_delay, this->signals[3], this->signals[4], n);
                this->dspexpr_06_perform(this->signals[4], this->signals[2], this->signals[5], n);
                this->signaladder_04_perform(this->signals[0], this->signals[5], this->signals[5], n);
                this->feedbackwriter_01_perform(this->signals[5], n);
                this->signalforwarder_02_perform(this->signals[3], out1, n);
                this->feedbackreader_02_perform(this->signals[3], n);
                this->delaytilde_02_perform(this->delaytilde_02_delay, this->signals[3], this->signals[0], n);
                this->dspexpr_08_perform(this->signals[0], this->signals[2], this->signals[4], n);
                this->signaladder_05_perform(this->signals[4], this->signals[1], this->signals[1], n);
                this->feedbackwriter_02_perform(this->signals[1], n);
                this->signalforwarder_03_perform(this->signals[3], out2, n);
                this->stackprotect_perform(n);
                this->audioProcessSampleCount += this->vs;
            }
            
            void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
                RNBO_ASSERT(this->_isInitialized);
            
                if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
                    Index i;
            
                    for (i = 0; i < 6; i++) {
                        this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
                    }
            
                    this->feedbacktilde_01_feedbackbuffer = resizeSignal(this->feedbacktilde_01_feedbackbuffer, this->maxvs, maxBlockSize);
                    this->feedbacktilde_02_feedbackbuffer = resizeSignal(this->feedbacktilde_02_feedbackbuffer, this->maxvs, maxBlockSize);
                    this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
                    this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
                    this->didAllocateSignals = true;
                }
            
                const bool sampleRateChanged = sampleRate != this->sr;
                const bool maxvsChanged = maxBlockSize != this->maxvs;
                const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;
            
                if (sampleRateChanged || maxvsChanged) {
                    this->vs = maxBlockSize;
                    this->maxvs = maxBlockSize;
                    this->sr = sampleRate;
                    this->invsr = 1 / sampleRate;
                }
            
                this->delaytilde_01_dspsetup(forceDSPSetup);
                this->delaytilde_02_dspsetup(forceDSPSetup);
            
                if (sampleRateChanged)
                    this->onSampleRateChanged(sampleRate);
            }
            
            number msToSamps(MillisecondTime ms, number sampleRate) {
                return ms * sampleRate * 0.001;
            }
            
            MillisecondTime sampsToMs(SampleIndex samps) {
                return samps * (this->invsr * 1000);
            }
            
            Index getNumInputChannels() const {
                return 1;
            }
            
            Index getNumOutputChannels() const {
                return 2;
            }
            
            void getPreset(PatcherStateInterface& preset) {
                this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
                this->param_05_getPresetValue(getSubState(preset, "left_delay"));
                this->param_06_getPresetValue(getSubState(preset, "fb"));
                this->param_07_getPresetValue(getSubState(preset, "right_delay"));
            }
            
            void setPreset(MillisecondTime , PatcherStateInterface& ) {}
            
            void setParameterValue(ParameterIndex index, ParameterValue v, MillisecondTime time) {
                this->updateTime(time, (ENGINE*)nullptr);
            
                switch (index) {
                case 0:
                    {
                    this->param_05_value_set(v);
                    break;
                    }
                case 1:
                    {
                    this->param_06_value_set(v);
                    break;
                    }
                case 2:
                    {
                    this->param_07_value_set(v);
                    break;
                    }
                }
            }
            
            void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                this->setParameterValue(index, value, time);
            }
            
            void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
                this->setParameterValue(index, this->getParameterValue(index), time);
            }
            
            void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                this->setParameterValueNormalized(index, value, time);
            }
            
            ParameterValue getParameterValue(ParameterIndex index)  {
                switch (index) {
                case 0:
                    {
                    return this->param_05_value;
                    }
                case 1:
                    {
                    return this->param_06_value;
                    }
                case 2:
                    {
                    return this->param_07_value;
                    }
                default:
                    {
                    return 0;
                    }
                }
            }
            
            ParameterIndex getNumSignalInParameters() const {
                return 0;
            }
            
            ParameterIndex getNumSignalOutParameters() const {
                return 0;
            }
            
            ParameterIndex getNumParameters() const {
                return 3;
            }
            
            ConstCharPointer getParameterName(ParameterIndex index) const {
                switch (index) {
                case 0:
                    {
                    return "left_delay";
                    }
                case 1:
                    {
                    return "fb";
                    }
                case 2:
                    {
                    return "right_delay";
                    }
                default:
                    {
                    return "bogus";
                    }
                }
            }
            
            ConstCharPointer getParameterId(ParameterIndex index) const {
                switch (index) {
                case 0:
                    {
                    return "poly/stereo_delay/left_delay";
                    }
                case 1:
                    {
                    return "poly/stereo_delay/fb";
                    }
                case 2:
                    {
                    return "poly/stereo_delay/right_delay";
                    }
                default:
                    {
                    return "bogus";
                    }
                }
            }
            
            void getParameterInfo(ParameterIndex index, ParameterInfo * info) const {
                {
                    switch (index) {
                    case 0:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 300;
                        info->min = 10;
                        info->max = 10000;
                        info->exponent = 1;
                        info->steps = 0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    case 1:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 0.25;
                        info->min = 0;
                        info->max = 1;
                        info->exponent = 1;
                        info->steps = 0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    case 2:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 400;
                        info->min = 10;
                        info->max = 10000;
                        info->exponent = 1;
                        info->steps = 0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    }
                }
            }
            
            ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
                if (steps == 1) {
                    if (normalizedValue > 0) {
                        normalizedValue = 1.;
                    }
                } else {
                    ParameterValue oneStep = (number)1. / (steps - 1);
                    ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
                    normalizedValue = numberOfSteps * oneStep;
                }
            
                return normalizedValue;
            }
            
            ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                switch (index) {
                case 1:
                    {
                    {
                        value = (value < 0 ? 0 : (value > 1 ? 1 : value));
                        ParameterValue normalizedValue = (value - 0) / (1 - 0);
                        return normalizedValue;
                    }
                    }
                case 0:
                case 2:
                    {
                    {
                        value = (value < 10 ? 10 : (value > 10000 ? 10000 : value));
                        ParameterValue normalizedValue = (value - 10) / (10000 - 10);
                        return normalizedValue;
                    }
                    }
                default:
                    {
                    return value;
                    }
                }
            }
            
            ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                value = (value < 0 ? 0 : (value > 1 ? 1 : value));
            
                switch (index) {
                case 1:
                    {
                    {
                        {
                            return 0 + value * (1 - 0);
                        }
                    }
                    }
                case 0:
                case 2:
                    {
                    {
                        {
                            return 10 + value * (10000 - 10);
                        }
                    }
                    }
                default:
                    {
                    return value;
                    }
                }
            }
            
            ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
                switch (index) {
                case 0:
                    {
                    return this->param_05_value_constrain(value);
                    }
                case 1:
                    {
                    return this->param_06_value_constrain(value);
                    }
                case 2:
                    {
                    return this->param_07_value_constrain(value);
                    }
                default:
                    {
                    return value;
                    }
                }
            }
            
            void processNumMessage(MessageTag , MessageTag , MillisecondTime , number ) {}
            
            void processListMessage(MessageTag , MessageTag , MillisecondTime , const list& ) {}
            
            void processBangMessage(MessageTag , MessageTag , MillisecondTime ) {}
            
            MessageTagInfo resolveTag(MessageTag tag) const {
                switch (tag) {
            
                }
            
                return nullptr;
            }
            
            DataRef* getDataRef(DataRefIndex index)  {
                switch (index) {
                case 2:
                    {
                    return addressOf(this->delaytilde_01_del_bufferobj);
                    break;
                    }
                case 3:
                    {
                    return addressOf(this->delaytilde_02_del_bufferobj);
                    break;
                    }
                default:
                    {
                    return nullptr;
                    }
                }
            }
            
            DataRefIndex getNumDataRefs() const {
                return 2;
            }
            
            void processDataViewUpdate(DataRefIndex index, MillisecondTime time) {
                this->updateTime(time, (ENGINE*)nullptr);
            
                if (index == 2) {
                    this->delaytilde_01_del_buffer = reInitDataView(this->delaytilde_01_del_buffer, this->delaytilde_01_del_bufferobj);
                }
            
                if (index == 3) {
                    this->delaytilde_02_del_buffer = reInitDataView(this->delaytilde_02_del_buffer, this->delaytilde_02_del_bufferobj);
                }
            }
            
            void initialize() {
                RNBO_ASSERT(!this->_isInitialized);
            
                this->delaytilde_01_del_bufferobj = initDataRef(
                    this->delaytilde_01_del_bufferobj,
                    this->dataRefStrings->name0,
                    true,
                    this->dataRefStrings->file0,
                    this->dataRefStrings->tag0
                );
            
                this->delaytilde_02_del_bufferobj = initDataRef(
                    this->delaytilde_02_del_bufferobj,
                    this->dataRefStrings->name1,
                    true,
                    this->dataRefStrings->file1,
                    this->dataRefStrings->tag1
                );
            
                this->assign_defaults();
                this->applyState();
                this->delaytilde_01_del_bufferobj->setIndex(2);
                this->delaytilde_01_del_buffer = new Float64Buffer(this->delaytilde_01_del_bufferobj);
                this->delaytilde_02_del_bufferobj->setIndex(3);
                this->delaytilde_02_del_buffer = new Float64Buffer(this->delaytilde_02_del_bufferobj);
                this->_isInitialized = true;
            }
            
            protected:
            
            void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
            	if (time == TimeNow) time = getTopLevelPatcher()->getPatcherTime();
            	getTopLevelPatcher()->processInternalEvents(time);
            	updateTime(time, (EXTERNALENGINE*)nullptr);
            }
            
            RNBOSubpatcher_951* operator->() {
                return this;
            }
            const RNBOSubpatcher_951* operator->() const {
                return this;
            }
            virtual RNBOSubpatcher_953* getPatcher() const {
                return static_cast<RNBOSubpatcher_953 *>(_parentPatcher);
            }
            
            rnbomatic* getTopLevelPatcher() {
                return this->getPatcher()->getTopLevelPatcher();
            }
            
            void cancelClockEvents()
            {
            }
            
            inline number linearinterp(number frac, number x, number y) {
                return x + (y - x) * frac;
            }
            
            inline number cubicinterp(number a, number w, number x, number y, number z) {
                number a1 = 1. + a;
                number aa = a * a1;
                number b = 1. - a;
                number b1 = 2. - a;
                number bb = b * b1;
                number fw = -.1666667 * bb * a;
                number fx = .5 * bb * a1;
                number fy = .5 * aa * b1;
                number fz = -.1666667 * aa * b;
                return w * fw + x * fx + y * fy + z * fz;
            }
            
            inline number fastcubicinterp(number a, number w, number x, number y, number z) {
                number a2 = a * a;
                number f0 = z - y - w + x;
                number f1 = w - x - f0;
                number f2 = y - w;
                number f3 = x;
                return f0 * a * a2 + f1 * a2 + f2 * a + f3;
            }
            
            inline number splineinterp(number a, number w, number x, number y, number z) {
                number a2 = a * a;
                number f0 = -0.5 * w + 1.5 * x - 1.5 * y + 0.5 * z;
                number f1 = w - 2.5 * x + 2 * y - 0.5 * z;
                number f2 = -0.5 * w + 0.5 * y;
                return f0 * a * a2 + f1 * a2 + f2 * a + x;
            }
            
            inline number spline6interp(number a, number y0, number y1, number y2, number y3, number y4, number y5) {
                number ym2py2 = y0 + y4;
                number ym1py1 = y1 + y3;
                number y2mym2 = y4 - y0;
                number y1mym1 = y3 - y1;
                number sixthym1py1 = (number)1 / (number)6.0 * ym1py1;
                number c0 = (number)1 / (number)120.0 * ym2py2 + (number)13 / (number)60.0 * ym1py1 + (number)11 / (number)20.0 * y2;
                number c1 = (number)1 / (number)24.0 * y2mym2 + (number)5 / (number)12.0 * y1mym1;
                number c2 = (number)1 / (number)12.0 * ym2py2 + sixthym1py1 - (number)1 / (number)2.0 * y2;
                number c3 = (number)1 / (number)12.0 * y2mym2 - (number)1 / (number)6.0 * y1mym1;
                number c4 = (number)1 / (number)24.0 * ym2py2 - sixthym1py1 + (number)1 / (number)4.0 * y2;
                number c5 = (number)1 / (number)120.0 * (y5 - y0) + (number)1 / (number)24.0 * (y1 - y4) + (number)1 / (number)12.0 * (y3 - y2);
                return ((((c5 * a + c4) * a + c3) * a + c2) * a + c1) * a + c0;
            }
            
            inline number cosT8(number r) {
                number t84 = 56.0;
                number t83 = 1680.0;
                number t82 = 20160.0;
                number t81 = 2.4801587302e-05;
                number t73 = 42.0;
                number t72 = 840.0;
                number t71 = 1.9841269841e-04;
            
                if (r < 0.785398163397448309615660845819875721 && r > -0.785398163397448309615660845819875721) {
                    number rr = r * r;
                    return 1.0 - rr * t81 * (t82 - rr * (t83 - rr * (t84 - rr)));
                } else if (r > 0.0) {
                    r -= 1.57079632679489661923132169163975144;
                    number rr = r * r;
                    return -r * (1.0 - t71 * rr * (t72 - rr * (t73 - rr)));
                } else {
                    r += 1.57079632679489661923132169163975144;
                    number rr = r * r;
                    return r * (1.0 - t71 * rr * (t72 - rr * (t73 - rr)));
                }
            }
            
            inline number cosineinterp(number frac, number x, number y) {
                number a2 = (1.0 - this->cosT8(frac * 3.14159265358979323846)) / (number)2.0;
                return x * (1.0 - a2) + y * a2;
            }
            
            number mstosamps(MillisecondTime ms) {
                return ms * this->sr * 0.001;
            }
            
            number maximum(number x, number y) {
                return (x < y ? y : x);
            }
            
            inline number safediv(number num, number denom) {
                return (denom == 0.0 ? 0.0 : num / denom);
            }
            
            number fromnormalized(Index index, number normalizedValue) {
                return this->convertFromNormalizedParameterValue(index, normalizedValue);
            }
            
            void param_05_value_set(number v) {
                v = this->param_05_value_constrain(v);
                this->param_05_value = v;
                this->sendParameter(0, false);
            
                if (this->param_05_value != this->param_05_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_05_lastValue = this->param_05_value;
                }
            
                this->mstosamps_01_ms_set(v);
            }
            
            void param_06_value_set(number v) {
                v = this->param_06_value_constrain(v);
                this->param_06_value = v;
                this->sendParameter(1, false);
            
                if (this->param_06_value != this->param_06_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_06_lastValue = this->param_06_value;
                }
            
                this->slide_tilde_01_x_set(v);
            }
            
            void param_07_value_set(number v) {
                v = this->param_07_value_constrain(v);
                this->param_07_value = v;
                this->sendParameter(2, false);
            
                if (this->param_07_value != this->param_07_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_07_lastValue = this->param_07_value;
                }
            
                this->mstosamps_02_ms_set(v);
            }
            
            MillisecondTime getPatcherTime() const {
                return this->_currentTime;
            }
            
            void deallocateSignals() {
                Index i;
            
                for (i = 0; i < 6; i++) {
                    this->signals[i] = freeSignal(this->signals[i]);
                }
            
                this->feedbacktilde_01_feedbackbuffer = freeSignal(this->feedbacktilde_01_feedbackbuffer);
                this->feedbacktilde_02_feedbackbuffer = freeSignal(this->feedbacktilde_02_feedbackbuffer);
                this->zeroBuffer = freeSignal(this->zeroBuffer);
                this->dummyBuffer = freeSignal(this->dummyBuffer);
            }
            
            Index getMaxBlockSize() const {
                return this->maxvs;
            }
            
            number getSampleRate() const {
                return this->sr;
            }
            
            bool hasFixedVectorSize() const {
                return false;
            }
            
            void setProbingTarget(MessageTag ) {}
            
            void initializeObjects() {
                this->delaytilde_01_del_init();
                this->delaytilde_02_del_init();
            }
            
            void setVoiceIndex(Index index)  {
                this->_voiceIndex = index;
            }
            
            void setNoteNumber(Int noteNumber)  {
                this->_noteNumber = noteNumber;
            }
            
            Index getIsMuted()  {
                return this->isMuted;
            }
            
            void setIsMuted(Index v)  {
                this->isMuted = v;
            }
            
            void onSampleRateChanged(double ) {}
            
            void extractState(PatcherStateInterface& ) {}
            
            void applyState() {}
            
            void setParameterOffset(ParameterIndex offset) {
                this->parameterOffset = offset;
            }
            
            void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}
            
            void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}
            
            void processOutletEvent(
                EngineLink* sender,
                OutletIndex index,
                ParameterValue value,
                MillisecondTime time
            ) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->processOutletAtCurrentTime(sender, index, value);
            }
            
            void sendOutlet(OutletIndex index, ParameterValue value) {
                this->getEngine()->sendOutlet(this, index, value);
            }
            
            void startup() {
                this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
            
                {
                    this->scheduleParamInit(0, 0);
                }
            
                {
                    this->scheduleParamInit(1, 0);
                }
            
                {
                    this->scheduleParamInit(2, 0);
                }
            }
            
            void fillDataRef(DataRefIndex , DataRef& ) {}
            
            void zeroDataRef(DataRef& ref) {
                ref->setZero();
            }
            
            void allocateDataRefs() {
                this->delaytilde_01_del_buffer = this->delaytilde_01_del_buffer->allocateIfNeeded();
            
                if (this->delaytilde_01_del_bufferobj->hasRequestedSize()) {
                    if (this->delaytilde_01_del_bufferobj->wantsFill())
                        this->zeroDataRef(this->delaytilde_01_del_bufferobj);
            
                    this->getEngine()->sendDataRefUpdated(2);
                }
            
                this->delaytilde_02_del_buffer = this->delaytilde_02_del_buffer->allocateIfNeeded();
            
                if (this->delaytilde_02_del_bufferobj->hasRequestedSize()) {
                    if (this->delaytilde_02_del_bufferobj->wantsFill())
                        this->zeroDataRef(this->delaytilde_02_del_bufferobj);
            
                    this->getEngine()->sendDataRefUpdated(3);
                }
            }
            
            number param_05_value_constrain(number v) const {
                v = (v > 10000 ? 10000 : (v < 10 ? 10 : v));
                return v;
            }
            
            void delaytilde_01_delay_set(number v) {
                this->delaytilde_01_delay = v;
            }
            
            void mstosamps_01_out1_set(number v) {
                this->delaytilde_01_delay_set(v);
            }
            
            void mstosamps_01_ms_set(number ms) {
                this->mstosamps_01_ms = ms;
            
                {
                    this->mstosamps_01_out1_set(ms * this->sr * 0.001);
                    return;
                }
            }
            
            number param_06_value_constrain(number v) const {
                v = (v > 1 ? 1 : (v < 0 ? 0 : v));
                return v;
            }
            
            void slide_tilde_01_x_set(number v) {
                this->slide_tilde_01_x = v;
            }
            
            number param_07_value_constrain(number v) const {
                v = (v > 10000 ? 10000 : (v < 10 ? 10 : v));
                return v;
            }
            
            void delaytilde_02_delay_set(number v) {
                this->delaytilde_02_delay = v;
            }
            
            void mstosamps_02_out1_set(number v) {
                this->delaytilde_02_delay_set(v);
            }
            
            void mstosamps_02_ms_set(number ms) {
                this->mstosamps_02_ms = ms;
            
                {
                    this->mstosamps_02_out1_set(ms * this->sr * 0.001);
                    return;
                }
            }
            
            void ctlin_05_outchannel_set(number ) {}
            
            void ctlin_05_outcontroller_set(number ) {}
            
            void fromnormalized_05_output_set(number v) {
                this->param_05_value_set(v);
            }
            
            void fromnormalized_05_input_set(number v) {
                this->fromnormalized_05_output_set(this->fromnormalized(0, v));
            }
            
            void expr_05_out1_set(number v) {
                this->expr_05_out1 = v;
                this->fromnormalized_05_input_set(this->expr_05_out1);
            }
            
            void expr_05_in1_set(number in1) {
                this->expr_05_in1 = in1;
                this->expr_05_out1_set(this->expr_05_in1 * this->expr_05_in2);//#map:expr_05:1
            }
            
            void ctlin_05_value_set(number v) {
                this->expr_05_in1_set(v);
            }
            
            void ctlin_05_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_05_channel || this->ctlin_05_channel == -1) && (data[1] == this->ctlin_05_controller || this->ctlin_05_controller == -1)) {
                    this->ctlin_05_outchannel_set(channel);
                    this->ctlin_05_outcontroller_set(data[1]);
                    this->ctlin_05_value_set(data[2]);
                    this->ctlin_05_status = 0;
                }
            }
            
            void ctlin_06_outchannel_set(number ) {}
            
            void ctlin_06_outcontroller_set(number ) {}
            
            void fromnormalized_06_output_set(number v) {
                this->param_06_value_set(v);
            }
            
            void fromnormalized_06_input_set(number v) {
                this->fromnormalized_06_output_set(this->fromnormalized(1, v));
            }
            
            void expr_06_out1_set(number v) {
                this->expr_06_out1 = v;
                this->fromnormalized_06_input_set(this->expr_06_out1);
            }
            
            void expr_06_in1_set(number in1) {
                this->expr_06_in1 = in1;
                this->expr_06_out1_set(this->expr_06_in1 * this->expr_06_in2);//#map:expr_06:1
            }
            
            void ctlin_06_value_set(number v) {
                this->expr_06_in1_set(v);
            }
            
            void ctlin_06_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_06_channel || this->ctlin_06_channel == -1) && (data[1] == this->ctlin_06_controller || this->ctlin_06_controller == -1)) {
                    this->ctlin_06_outchannel_set(channel);
                    this->ctlin_06_outcontroller_set(data[1]);
                    this->ctlin_06_value_set(data[2]);
                    this->ctlin_06_status = 0;
                }
            }
            
            void ctlin_07_outchannel_set(number ) {}
            
            void ctlin_07_outcontroller_set(number ) {}
            
            void fromnormalized_07_output_set(number v) {
                this->param_07_value_set(v);
            }
            
            void fromnormalized_07_input_set(number v) {
                this->fromnormalized_07_output_set(this->fromnormalized(2, v));
            }
            
            void expr_07_out1_set(number v) {
                this->expr_07_out1 = v;
                this->fromnormalized_07_input_set(this->expr_07_out1);
            }
            
            void expr_07_in1_set(number in1) {
                this->expr_07_in1 = in1;
                this->expr_07_out1_set(this->expr_07_in1 * this->expr_07_in2);//#map:expr_07:1
            }
            
            void ctlin_07_value_set(number v) {
                this->expr_07_in1_set(v);
            }
            
            void ctlin_07_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_07_channel || this->ctlin_07_channel == -1) && (data[1] == this->ctlin_07_controller || this->ctlin_07_controller == -1)) {
                    this->ctlin_07_outchannel_set(channel);
                    this->ctlin_07_outcontroller_set(data[1]);
                    this->ctlin_07_value_set(data[2]);
                    this->ctlin_07_status = 0;
                }
            }
            
            void dspexpr_07_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
                RNBO_UNUSED(in2);
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out1[(Index)i] = in1[(Index)i] * 0.3;//#map:_###_obj_###_:1
                }
            }
            
            void dspexpr_09_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
                RNBO_UNUSED(in2);
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out1[(Index)i] = in1[(Index)i] * 0.3;//#map:_###_obj_###_:1
                }
            }
            
            void slide_tilde_01_perform(number x, number up, number down, SampleValue * out1, Index n) {
                RNBO_UNUSED(down);
                RNBO_UNUSED(up);
                auto __slide_tilde_01_prev = this->slide_tilde_01_prev;
                auto iup = this->safediv(1., this->maximum(1., rnbo_abs(10)));
                auto idown = this->safediv(1., this->maximum(1., rnbo_abs(10)));
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    number temp = x - __slide_tilde_01_prev;
                    __slide_tilde_01_prev = __slide_tilde_01_prev + ((x > __slide_tilde_01_prev ? iup : idown)) * temp;
                    out1[(Index)i] = __slide_tilde_01_prev;
                }
            
                this->slide_tilde_01_prev = __slide_tilde_01_prev;
            }
            
            void feedbackreader_01_perform(SampleValue * output, Index n) {
                auto& buffer = this->feedbacktilde_01_feedbackbuffer;
            
                for (Index i = 0; i < n; i++) {
                    output[(Index)i] = buffer[(Index)i];
                }
            }
            
            void delaytilde_01_perform(number delay, const SampleValue * input, SampleValue * output, Index n) {
                auto __delaytilde_01_crossfadeDelay = this->delaytilde_01_crossfadeDelay;
                auto __delaytilde_01_rampInSamples = this->delaytilde_01_rampInSamples;
                auto __delaytilde_01_ramp = this->delaytilde_01_ramp;
                auto __delaytilde_01_lastDelay = this->delaytilde_01_lastDelay;
            
                for (Index i = 0; i < n; i++) {
                    if (__delaytilde_01_lastDelay == -1) {
                        __delaytilde_01_lastDelay = delay;
                    }
            
                    if (__delaytilde_01_ramp > 0) {
                        number factor = __delaytilde_01_ramp / __delaytilde_01_rampInSamples;
                        output[(Index)i] = this->delaytilde_01_del_read(__delaytilde_01_crossfadeDelay, 0) * factor + this->delaytilde_01_del_read(__delaytilde_01_lastDelay, 0) * (1. - factor);
                        __delaytilde_01_ramp--;
                    } else {
                        number effectiveDelay = delay;
            
                        if (effectiveDelay != __delaytilde_01_lastDelay) {
                            __delaytilde_01_ramp = __delaytilde_01_rampInSamples;
                            __delaytilde_01_crossfadeDelay = __delaytilde_01_lastDelay;
                            __delaytilde_01_lastDelay = effectiveDelay;
                            output[(Index)i] = this->delaytilde_01_del_read(__delaytilde_01_crossfadeDelay, 0);
                            __delaytilde_01_ramp--;
                        } else {
                            output[(Index)i] = this->delaytilde_01_del_read(effectiveDelay, 0);
                        }
                    }
            
                    this->delaytilde_01_del_write(input[(Index)i]);
                    this->delaytilde_01_del_step();
                }
            
                this->delaytilde_01_lastDelay = __delaytilde_01_lastDelay;
                this->delaytilde_01_ramp = __delaytilde_01_ramp;
                this->delaytilde_01_crossfadeDelay = __delaytilde_01_crossfadeDelay;
            }
            
            void dspexpr_06_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
                }
            }
            
            void signaladder_04_perform(
                const SampleValue * in1,
                const SampleValue * in2,
                SampleValue * out,
                Index n
            ) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out[(Index)i] = in1[(Index)i] + in2[(Index)i];
                }
            }
            
            void feedbackwriter_01_perform(const SampleValue * input, Index n) {
                auto& buffer = this->feedbacktilde_01_feedbackbuffer;
            
                for (Index i = 0; i < n; i++) {
                    buffer[(Index)i] = input[(Index)i];
                }
            }
            
            void signalforwarder_02_perform(const SampleValue * input, SampleValue * output, Index n) {
                copySignal(output, input, n);
            }
            
            void feedbackreader_02_perform(SampleValue * output, Index n) {
                auto& buffer = this->feedbacktilde_02_feedbackbuffer;
            
                for (Index i = 0; i < n; i++) {
                    output[(Index)i] = buffer[(Index)i];
                }
            }
            
            void delaytilde_02_perform(number delay, const SampleValue * input, SampleValue * output, Index n) {
                auto __delaytilde_02_crossfadeDelay = this->delaytilde_02_crossfadeDelay;
                auto __delaytilde_02_rampInSamples = this->delaytilde_02_rampInSamples;
                auto __delaytilde_02_ramp = this->delaytilde_02_ramp;
                auto __delaytilde_02_lastDelay = this->delaytilde_02_lastDelay;
            
                for (Index i = 0; i < n; i++) {
                    if (__delaytilde_02_lastDelay == -1) {
                        __delaytilde_02_lastDelay = delay;
                    }
            
                    if (__delaytilde_02_ramp > 0) {
                        number factor = __delaytilde_02_ramp / __delaytilde_02_rampInSamples;
                        output[(Index)i] = this->delaytilde_02_del_read(__delaytilde_02_crossfadeDelay, 0) * factor + this->delaytilde_02_del_read(__delaytilde_02_lastDelay, 0) * (1. - factor);
                        __delaytilde_02_ramp--;
                    } else {
                        number effectiveDelay = delay;
            
                        if (effectiveDelay != __delaytilde_02_lastDelay) {
                            __delaytilde_02_ramp = __delaytilde_02_rampInSamples;
                            __delaytilde_02_crossfadeDelay = __delaytilde_02_lastDelay;
                            __delaytilde_02_lastDelay = effectiveDelay;
                            output[(Index)i] = this->delaytilde_02_del_read(__delaytilde_02_crossfadeDelay, 0);
                            __delaytilde_02_ramp--;
                        } else {
                            output[(Index)i] = this->delaytilde_02_del_read(effectiveDelay, 0);
                        }
                    }
            
                    this->delaytilde_02_del_write(input[(Index)i]);
                    this->delaytilde_02_del_step();
                }
            
                this->delaytilde_02_lastDelay = __delaytilde_02_lastDelay;
                this->delaytilde_02_ramp = __delaytilde_02_ramp;
                this->delaytilde_02_crossfadeDelay = __delaytilde_02_crossfadeDelay;
            }
            
            void dspexpr_08_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
                }
            }
            
            void signaladder_05_perform(
                const SampleValue * in1,
                const SampleValue * in2,
                SampleValue * out,
                Index n
            ) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out[(Index)i] = in1[(Index)i] + in2[(Index)i];
                }
            }
            
            void feedbackwriter_02_perform(const SampleValue * input, Index n) {
                auto& buffer = this->feedbacktilde_02_feedbackbuffer;
            
                for (Index i = 0; i < n; i++) {
                    buffer[(Index)i] = input[(Index)i];
                }
            }
            
            void signalforwarder_03_perform(const SampleValue * input, SampleValue * output, Index n) {
                copySignal(output, input, n);
            }
            
            void stackprotect_perform(Index n) {
                RNBO_UNUSED(n);
                auto __stackprotect_count = this->stackprotect_count;
                __stackprotect_count = 0;
                this->stackprotect_count = __stackprotect_count;
            }
            
            void delaytilde_01_del_step() {
                this->delaytilde_01_del_reader++;
            
                if (this->delaytilde_01_del_reader >= (Int)(this->delaytilde_01_del_buffer->getSize()))
                    this->delaytilde_01_del_reader = 0;
            }
            
            number delaytilde_01_del_read(number size, Int interp) {
                if (interp == 0) {
                    number r = (Int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Int index2 = (Int)(index1 + 1);
            
                    return this->linearinterp(frac, this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ));
                } else if (interp == 1) {
                    number r = (Int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? 1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Index index2 = (Index)(index1 + 1);
                    Index index3 = (Index)(index2 + 1);
                    Index index4 = (Index)(index3 + 1);
            
                    return this->cubicinterp(frac, this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index3 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index4 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ));
                } else if (interp == 6) {
                    number r = (Int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? 1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Index index2 = (Index)(index1 + 1);
                    Index index3 = (Index)(index2 + 1);
                    Index index4 = (Index)(index3 + 1);
            
                    return this->fastcubicinterp(frac, this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index3 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index4 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ));
                } else if (interp == 2) {
                    number r = (Int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? 1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Index index2 = (Index)(index1 + 1);
                    Index index3 = (Index)(index2 + 1);
                    Index index4 = (Index)(index3 + 1);
            
                    return this->splineinterp(frac, this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index3 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index4 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ));
                } else if (interp == 7) {
                    number r = (Int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? 1 + this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Index index2 = (Index)(index1 + 1);
                    Index index3 = (Index)(index2 + 1);
                    Index index4 = (Index)(index3 + 1);
                    Index index5 = (Index)(index4 + 1);
                    Index index6 = (Index)(index5 + 1);
            
                    return this->spline6interp(frac, this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index3 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index4 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index5 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index6 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ));
                } else if (interp == 3) {
                    number r = (Int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Index index2 = (Index)(index1 + 1);
            
                    return this->cosineinterp(frac, this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ), this->delaytilde_01_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_01_del_wrap))
                    ));
                }
            
                number r = (Int)(this->delaytilde_01_del_buffer->getSize()) + this->delaytilde_01_del_reader - ((size > this->delaytilde_01_del__maxdelay ? this->delaytilde_01_del__maxdelay : (size < (this->delaytilde_01_del_reader != this->delaytilde_01_del_writer) ? this->delaytilde_01_del_reader != this->delaytilde_01_del_writer : size)));
                Int index1 = (Int)(rnbo_floor(r));
            
                return this->delaytilde_01_del_buffer->getSample(
                    0,
                    (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_01_del_wrap))
                );
            }
            
            void delaytilde_01_del_write(number v) {
                this->delaytilde_01_del_writer = this->delaytilde_01_del_reader;
                this->delaytilde_01_del_buffer[(Index)this->delaytilde_01_del_writer] = v;
            }
            
            number delaytilde_01_del_next(number v, Int size) {
                number effectiveSize = (size == -1 ? this->delaytilde_01_del__maxdelay : size);
                number val = this->delaytilde_01_del_read(effectiveSize, 0);
                this->delaytilde_01_del_write(v);
                this->delaytilde_01_del_step();
                return val;
            }
            
            array<Index, 2> delaytilde_01_del_calcSizeInSamples() {
                number sizeInSamples = 0;
                Index allocatedSizeInSamples = 0;
            
                {
                    sizeInSamples = this->delaytilde_01_del_evaluateSizeExpr(this->sr, this->vs);
                    this->delaytilde_01_del_sizemode = 0;
                }
            
                sizeInSamples = rnbo_floor(sizeInSamples);
                sizeInSamples = this->maximum(sizeInSamples, 2);
                allocatedSizeInSamples = (Index)(sizeInSamples);
                allocatedSizeInSamples = nextpoweroftwo(allocatedSizeInSamples);
                return {sizeInSamples, allocatedSizeInSamples};
            }
            
            void delaytilde_01_del_init() {
                auto result = this->delaytilde_01_del_calcSizeInSamples();
                this->delaytilde_01_del__maxdelay = result[0];
                Index requestedSizeInSamples = (Index)(result[1]);
                this->delaytilde_01_del_buffer->requestSize(requestedSizeInSamples, 1);
                this->delaytilde_01_del_wrap = requestedSizeInSamples - 1;
            }
            
            void delaytilde_01_del_clear() {
                this->delaytilde_01_del_buffer->setZero();
            }
            
            void delaytilde_01_del_reset() {
                auto result = this->delaytilde_01_del_calcSizeInSamples();
                this->delaytilde_01_del__maxdelay = result[0];
                Index allocatedSizeInSamples = (Index)(result[1]);
                this->delaytilde_01_del_buffer->setSize(allocatedSizeInSamples);
                updateDataRef(this, this->delaytilde_01_del_buffer);
                this->delaytilde_01_del_wrap = this->delaytilde_01_del_buffer->getSize() - 1;
                this->delaytilde_01_del_clear();
            
                if (this->delaytilde_01_del_reader >= this->delaytilde_01_del__maxdelay || this->delaytilde_01_del_writer >= this->delaytilde_01_del__maxdelay) {
                    this->delaytilde_01_del_reader = 0;
                    this->delaytilde_01_del_writer = 0;
                }
            }
            
            void delaytilde_01_del_dspsetup() {
                this->delaytilde_01_del_reset();
            }
            
            number delaytilde_01_del_evaluateSizeExpr(number samplerate, number vectorsize) {
                RNBO_UNUSED(vectorsize);
                return samplerate;
            }
            
            number delaytilde_01_del_size() {
                return this->delaytilde_01_del__maxdelay;
            }
            
            void delaytilde_01_dspsetup(bool force) {
                if ((bool)(this->delaytilde_01_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->delaytilde_01_rampInSamples = (Int)(this->mstosamps(50));
                this->delaytilde_01_lastDelay = -1;
                this->delaytilde_01_setupDone = true;
                this->delaytilde_01_del_dspsetup();
            }
            
            void param_05_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_05_value;
            }
            
            void param_05_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_05_value_set(preset["value"]);
            }
            
            void param_06_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_06_value;
            }
            
            void param_06_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_06_value_set(preset["value"]);
            }
            
            void param_07_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_07_value;
            }
            
            void param_07_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_07_value_set(preset["value"]);
            }
            
            void delaytilde_02_del_step() {
                this->delaytilde_02_del_reader++;
            
                if (this->delaytilde_02_del_reader >= (Int)(this->delaytilde_02_del_buffer->getSize()))
                    this->delaytilde_02_del_reader = 0;
            }
            
            number delaytilde_02_del_read(number size, Int interp) {
                if (interp == 0) {
                    number r = (Int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Int index2 = (Int)(index1 + 1);
            
                    return this->linearinterp(frac, this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ));
                } else if (interp == 1) {
                    number r = (Int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? 1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Index index2 = (Index)(index1 + 1);
                    Index index3 = (Index)(index2 + 1);
                    Index index4 = (Index)(index3 + 1);
            
                    return this->cubicinterp(frac, this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index3 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index4 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ));
                } else if (interp == 6) {
                    number r = (Int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? 1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Index index2 = (Index)(index1 + 1);
                    Index index3 = (Index)(index2 + 1);
                    Index index4 = (Index)(index3 + 1);
            
                    return this->fastcubicinterp(frac, this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index3 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index4 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ));
                } else if (interp == 2) {
                    number r = (Int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? 1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Index index2 = (Index)(index1 + 1);
                    Index index3 = (Index)(index2 + 1);
                    Index index4 = (Index)(index3 + 1);
            
                    return this->splineinterp(frac, this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index3 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index4 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ));
                } else if (interp == 7) {
                    number r = (Int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? 1 + this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Index index2 = (Index)(index1 + 1);
                    Index index3 = (Index)(index2 + 1);
                    Index index4 = (Index)(index3 + 1);
                    Index index5 = (Index)(index4 + 1);
                    Index index6 = (Index)(index5 + 1);
            
                    return this->spline6interp(frac, this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index3 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index4 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index5 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index6 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ));
                } else if (interp == 3) {
                    number r = (Int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
                    Int index1 = (Int)(rnbo_floor(r));
                    number frac = r - index1;
                    Index index2 = (Index)(index1 + 1);
            
                    return this->cosineinterp(frac, this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ), this->delaytilde_02_del_buffer->getSample(
                        0,
                        (Index)((BinOpInt)((BinOpInt)index2 & (BinOpInt)this->delaytilde_02_del_wrap))
                    ));
                }
            
                number r = (Int)(this->delaytilde_02_del_buffer->getSize()) + this->delaytilde_02_del_reader - ((size > this->delaytilde_02_del__maxdelay ? this->delaytilde_02_del__maxdelay : (size < (this->delaytilde_02_del_reader != this->delaytilde_02_del_writer) ? this->delaytilde_02_del_reader != this->delaytilde_02_del_writer : size)));
                Int index1 = (Int)(rnbo_floor(r));
            
                return this->delaytilde_02_del_buffer->getSample(
                    0,
                    (Index)((BinOpInt)((BinOpInt)index1 & (BinOpInt)this->delaytilde_02_del_wrap))
                );
            }
            
            void delaytilde_02_del_write(number v) {
                this->delaytilde_02_del_writer = this->delaytilde_02_del_reader;
                this->delaytilde_02_del_buffer[(Index)this->delaytilde_02_del_writer] = v;
            }
            
            number delaytilde_02_del_next(number v, Int size) {
                number effectiveSize = (size == -1 ? this->delaytilde_02_del__maxdelay : size);
                number val = this->delaytilde_02_del_read(effectiveSize, 0);
                this->delaytilde_02_del_write(v);
                this->delaytilde_02_del_step();
                return val;
            }
            
            array<Index, 2> delaytilde_02_del_calcSizeInSamples() {
                number sizeInSamples = 0;
                Index allocatedSizeInSamples = 0;
            
                {
                    sizeInSamples = this->delaytilde_02_del_evaluateSizeExpr(this->sr, this->vs);
                    this->delaytilde_02_del_sizemode = 0;
                }
            
                sizeInSamples = rnbo_floor(sizeInSamples);
                sizeInSamples = this->maximum(sizeInSamples, 2);
                allocatedSizeInSamples = (Index)(sizeInSamples);
                allocatedSizeInSamples = nextpoweroftwo(allocatedSizeInSamples);
                return {sizeInSamples, allocatedSizeInSamples};
            }
            
            void delaytilde_02_del_init() {
                auto result = this->delaytilde_02_del_calcSizeInSamples();
                this->delaytilde_02_del__maxdelay = result[0];
                Index requestedSizeInSamples = (Index)(result[1]);
                this->delaytilde_02_del_buffer->requestSize(requestedSizeInSamples, 1);
                this->delaytilde_02_del_wrap = requestedSizeInSamples - 1;
            }
            
            void delaytilde_02_del_clear() {
                this->delaytilde_02_del_buffer->setZero();
            }
            
            void delaytilde_02_del_reset() {
                auto result = this->delaytilde_02_del_calcSizeInSamples();
                this->delaytilde_02_del__maxdelay = result[0];
                Index allocatedSizeInSamples = (Index)(result[1]);
                this->delaytilde_02_del_buffer->setSize(allocatedSizeInSamples);
                updateDataRef(this, this->delaytilde_02_del_buffer);
                this->delaytilde_02_del_wrap = this->delaytilde_02_del_buffer->getSize() - 1;
                this->delaytilde_02_del_clear();
            
                if (this->delaytilde_02_del_reader >= this->delaytilde_02_del__maxdelay || this->delaytilde_02_del_writer >= this->delaytilde_02_del__maxdelay) {
                    this->delaytilde_02_del_reader = 0;
                    this->delaytilde_02_del_writer = 0;
                }
            }
            
            void delaytilde_02_del_dspsetup() {
                this->delaytilde_02_del_reset();
            }
            
            number delaytilde_02_del_evaluateSizeExpr(number samplerate, number vectorsize) {
                RNBO_UNUSED(vectorsize);
                return samplerate;
            }
            
            number delaytilde_02_del_size() {
                return this->delaytilde_02_del__maxdelay;
            }
            
            void delaytilde_02_dspsetup(bool force) {
                if ((bool)(this->delaytilde_02_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->delaytilde_02_rampInSamples = (Int)(this->mstosamps(50));
                this->delaytilde_02_lastDelay = -1;
                this->delaytilde_02_setupDone = true;
                this->delaytilde_02_del_dspsetup();
            }
            
            bool stackprotect_check() {
                this->stackprotect_count++;
            
                if (this->stackprotect_count > 128) {
                    console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
                    return true;
                }
            
                return false;
            }
            
            Index getPatcherSerial() const {
                return 0;
            }
            
            void sendParameter(ParameterIndex index, bool ignoreValue) {
                this->getPatcher()->sendParameter(index + this->parameterOffset, ignoreValue);
            }
            
            void scheduleParamInit(ParameterIndex index, Index order) {
                this->getPatcher()->scheduleParamInit(index + this->parameterOffset, order);
            }
            
            void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
                RNBO_UNUSED(inProcess);
                RNBO_UNUSED(engine);
                this->_currentTime = time;
                auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));
            
                if (offset >= (SampleIndex)(this->vs))
                    offset = (SampleIndex)(this->vs) - 1;
            
                if (offset < 0)
                    offset = 0;
            
                this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
            }
            
            void assign_defaults()
            {
                dspexpr_06_in1 = 0;
                dspexpr_06_in2 = 0.25;
                delaytilde_01_delay = 0;
                param_05_value = 300;
                mstosamps_01_ms = 0;
                dspexpr_07_in1 = 0;
                dspexpr_07_in2 = 0.3;
                slide_tilde_01_x = 0;
                slide_tilde_01_up = 10;
                slide_tilde_01_down = 10;
                param_06_value = 0.25;
                param_07_value = 400;
                mstosamps_02_ms = 0;
                dspexpr_08_in1 = 0;
                dspexpr_08_in2 = 0.25;
                delaytilde_02_delay = 0;
                dspexpr_09_in1 = 0;
                dspexpr_09_in2 = 0.3;
                ctlin_05_input = 0;
                ctlin_05_controller = 0;
                ctlin_05_channel = -1;
                expr_05_in1 = 0;
                expr_05_in2 = 0.007874015748;
                expr_05_out1 = 0;
                ctlin_06_input = 0;
                ctlin_06_controller = 0;
                ctlin_06_channel = -1;
                expr_06_in1 = 0;
                expr_06_in2 = 0.007874015748;
                expr_06_out1 = 0;
                ctlin_07_input = 0;
                ctlin_07_controller = 0;
                ctlin_07_channel = -1;
                expr_07_in1 = 0;
                expr_07_in2 = 0.007874015748;
                expr_07_out1 = 0;
                _currentTime = 0;
                audioProcessSampleCount = 0;
                sampleOffsetIntoNextAudioBuffer = 0;
                zeroBuffer = nullptr;
                dummyBuffer = nullptr;
                signals[0] = nullptr;
                signals[1] = nullptr;
                signals[2] = nullptr;
                signals[3] = nullptr;
                signals[4] = nullptr;
                signals[5] = nullptr;
                didAllocateSignals = 0;
                vs = 0;
                maxvs = 0;
                sr = 44100;
                invsr = 0.000022675736961451248;
                delaytilde_01_lastDelay = -1;
                delaytilde_01_crossfadeDelay = 0;
                delaytilde_01_ramp = 0;
                delaytilde_01_rampInSamples = 0;
                delaytilde_01_del__maxdelay = 0;
                delaytilde_01_del_sizemode = 0;
                delaytilde_01_del_wrap = 0;
                delaytilde_01_del_reader = 0;
                delaytilde_01_del_writer = 0;
                delaytilde_01_setupDone = false;
                param_05_lastValue = 0;
                feedbacktilde_01_feedbackbuffer = nullptr;
                slide_tilde_01_prev = 0;
                param_06_lastValue = 0;
                param_07_lastValue = 0;
                delaytilde_02_lastDelay = -1;
                delaytilde_02_crossfadeDelay = 0;
                delaytilde_02_ramp = 0;
                delaytilde_02_rampInSamples = 0;
                delaytilde_02_del__maxdelay = 0;
                delaytilde_02_del_sizemode = 0;
                delaytilde_02_del_wrap = 0;
                delaytilde_02_del_reader = 0;
                delaytilde_02_del_writer = 0;
                delaytilde_02_setupDone = false;
                feedbacktilde_02_feedbackbuffer = nullptr;
                ctlin_05_status = 0;
                ctlin_05_byte1 = -1;
                ctlin_05_inchan = 0;
                ctlin_06_status = 0;
                ctlin_06_byte1 = -1;
                ctlin_06_inchan = 0;
                ctlin_07_status = 0;
                ctlin_07_byte1 = -1;
                ctlin_07_inchan = 0;
                stackprotect_count = 0;
                _voiceIndex = 0;
                _noteNumber = 0;
                isMuted = 1;
                parameterOffset = 0;
            }
            
                // data ref strings
                struct DataRefStrings {
                	static constexpr auto& name0 = "delaytilde_01_del_bufferobj";
                	static constexpr auto& file0 = "";
                	static constexpr auto& tag0 = "buffer~";
                	static constexpr auto& name1 = "delaytilde_02_del_bufferobj";
                	static constexpr auto& file1 = "";
                	static constexpr auto& tag1 = "buffer~";
                	DataRefStrings* operator->() { return this; }
                	const DataRefStrings* operator->() const { return this; }
                };
            
                DataRefStrings dataRefStrings;
            
            // member variables
            
                number dspexpr_06_in1;
                number dspexpr_06_in2;
                number delaytilde_01_delay;
                number param_05_value;
                number mstosamps_01_ms;
                number dspexpr_07_in1;
                number dspexpr_07_in2;
                number slide_tilde_01_x;
                number slide_tilde_01_up;
                number slide_tilde_01_down;
                number param_06_value;
                number param_07_value;
                number mstosamps_02_ms;
                number dspexpr_08_in1;
                number dspexpr_08_in2;
                number delaytilde_02_delay;
                number dspexpr_09_in1;
                number dspexpr_09_in2;
                number ctlin_05_input;
                number ctlin_05_controller;
                number ctlin_05_channel;
                number expr_05_in1;
                number expr_05_in2;
                number expr_05_out1;
                number ctlin_06_input;
                number ctlin_06_controller;
                number ctlin_06_channel;
                number expr_06_in1;
                number expr_06_in2;
                number expr_06_out1;
                number ctlin_07_input;
                number ctlin_07_controller;
                number ctlin_07_channel;
                number expr_07_in1;
                number expr_07_in2;
                number expr_07_out1;
                MillisecondTime _currentTime;
                UInt64 audioProcessSampleCount;
                Index sampleOffsetIntoNextAudioBuffer;
                signal zeroBuffer;
                signal dummyBuffer;
                SampleValue * signals[6];
                bool didAllocateSignals;
                Index vs;
                Index maxvs;
                number sr;
                number invsr;
                number delaytilde_01_lastDelay;
                number delaytilde_01_crossfadeDelay;
                number delaytilde_01_ramp;
                Int delaytilde_01_rampInSamples;
                Float64BufferRef delaytilde_01_del_buffer;
                Index delaytilde_01_del__maxdelay;
                Int delaytilde_01_del_sizemode;
                Index delaytilde_01_del_wrap;
                Int delaytilde_01_del_reader;
                Int delaytilde_01_del_writer;
                bool delaytilde_01_setupDone;
                number param_05_lastValue;
                signal feedbacktilde_01_feedbackbuffer;
                number slide_tilde_01_prev;
                number param_06_lastValue;
                number param_07_lastValue;
                number delaytilde_02_lastDelay;
                number delaytilde_02_crossfadeDelay;
                number delaytilde_02_ramp;
                Int delaytilde_02_rampInSamples;
                Float64BufferRef delaytilde_02_del_buffer;
                Index delaytilde_02_del__maxdelay;
                Int delaytilde_02_del_sizemode;
                Index delaytilde_02_del_wrap;
                Int delaytilde_02_del_reader;
                Int delaytilde_02_del_writer;
                bool delaytilde_02_setupDone;
                signal feedbacktilde_02_feedbackbuffer;
                Int ctlin_05_status;
                Int ctlin_05_byte1;
                Int ctlin_05_inchan;
                Int ctlin_06_status;
                Int ctlin_06_byte1;
                Int ctlin_06_inchan;
                Int ctlin_07_status;
                Int ctlin_07_byte1;
                Int ctlin_07_inchan;
                number stackprotect_count;
                DataRef delaytilde_01_del_bufferobj;
                DataRef delaytilde_02_del_bufferobj;
                Index _voiceIndex;
                Int _noteNumber;
                Index isMuted;
                ParameterIndex parameterOffset;
                bool _isInitialized = false;
    };
    
    class RNBOSubpatcher_952 : public PatcherInterfaceImpl {
            
            friend class RNBOSubpatcher_953;
            friend class rnbomatic;
            
            public:
            
            RNBOSubpatcher_952()
            {}
            
            ~RNBOSubpatcher_952()
            {
                deallocateSignals();
            }
            
            Index getNumMidiInputPorts() const {
                return 1;
            }
            
            void processMidiEvent(MillisecondTime time, int port, ConstByteArray data, Index length) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->ctlin_08_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
                this->ctlin_09_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
                this->ctlin_10_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
                this->ctlin_11_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
                this->ctlin_12_midihandler(data[0] & 240, (data[0] & 15) + 1, port, data, length);
            }
            
            Index getNumMidiOutputPorts() const {
                return 0;
            }
            
            void process(
                const SampleValue * const* inputs,
                Index numInputs,
                SampleValue * const* outputs,
                Index numOutputs,
                Index n
            ) {
                this->vs = n;
                this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
                SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
                const SampleValue * in1 = (numInputs >= 1 && inputs[0] ? inputs[0] : this->zeroBuffer);
                this->ip_05_perform(this->signals[0], n);
                this->ip_06_perform(this->signals[1], n);
                this->ip_07_perform(this->signals[2], n);
                this->phasor_03_perform(in1, this->signals[3], n);
                this->p_03_perform(in1, this->signals[4], n);
                this->p_04_perform(in1, this->signals[5], n);
                this->phasor_04_perform(in1, this->signals[6], n);
                this->triangle_tilde_01_perform(this->signals[6], this->triangle_tilde_01_duty, this->signals[7], n);
            
                this->cycle_tilde_03_perform(
                    in1,
                    this->cycle_tilde_03_phase_offset,
                    this->signals[6],
                    this->dummyBuffer,
                    n
                );
            
                this->scale_tilde_01_perform(
                    this->signals[6],
                    this->scale_tilde_01_lowin,
                    this->scale_tilde_01_hiin,
                    this->scale_tilde_01_lowout,
                    this->scale_tilde_01_highout,
                    this->scale_tilde_01_pow,
                    this->signals[8],
                    n
                );
            
                this->selector_03_perform(
                    this->selector_03_onoff,
                    this->signals[8],
                    this->signals[3],
                    this->signals[4],
                    this->signals[5],
                    this->signals[7],
                    this->signals[6],
                    n
                );
            
                this->p_02_perform(this->signals[6], this->signals[1], this->signals[7], n);
                this->p_01_perform(this->signals[7], this->signals[2], this->signals[1], n);
                this->dspexpr_17_perform(this->signals[1], this->signals[0], out1, n);
                this->stackprotect_perform(n);
                this->audioProcessSampleCount += this->vs;
            }
            
            void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
                RNBO_ASSERT(this->_isInitialized);
            
                if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
                    Index i;
            
                    for (i = 0; i < 9; i++) {
                        this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
                    }
            
                    this->ip_05_sigbuf = resizeSignal(this->ip_05_sigbuf, this->maxvs, maxBlockSize);
                    this->ip_06_sigbuf = resizeSignal(this->ip_06_sigbuf, this->maxvs, maxBlockSize);
                    this->ip_07_sigbuf = resizeSignal(this->ip_07_sigbuf, this->maxvs, maxBlockSize);
                    this->phasor_03_sigbuf = resizeSignal(this->phasor_03_sigbuf, this->maxvs, maxBlockSize);
                    this->phasor_04_sigbuf = resizeSignal(this->phasor_04_sigbuf, this->maxvs, maxBlockSize);
                    this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
                    this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
                    this->didAllocateSignals = true;
                }
            
                const bool sampleRateChanged = sampleRate != this->sr;
                const bool maxvsChanged = maxBlockSize != this->maxvs;
                const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;
            
                if (sampleRateChanged || maxvsChanged) {
                    this->vs = maxBlockSize;
                    this->maxvs = maxBlockSize;
                    this->sr = sampleRate;
                    this->invsr = 1 / sampleRate;
                }
            
                this->ip_05_dspsetup(forceDSPSetup);
                this->ip_06_dspsetup(forceDSPSetup);
                this->ip_07_dspsetup(forceDSPSetup);
                this->phasor_03_dspsetup(forceDSPSetup);
                this->phasor_04_dspsetup(forceDSPSetup);
                this->cycle_tilde_03_dspsetup(forceDSPSetup);
                this->p_01->prepareToProcess(sampleRate, maxBlockSize, force);
                this->p_02->prepareToProcess(sampleRate, maxBlockSize, force);
                this->p_03->prepareToProcess(sampleRate, maxBlockSize, force);
                this->p_04->prepareToProcess(sampleRate, maxBlockSize, force);
            
                if (sampleRateChanged)
                    this->onSampleRateChanged(sampleRate);
            }
            
            number msToSamps(MillisecondTime ms, number sampleRate) {
                return ms * sampleRate * 0.001;
            }
            
            MillisecondTime sampsToMs(SampleIndex samps) {
                return samps * (this->invsr * 1000);
            }
            
            Index getNumInputChannels() const {
                return 1;
            }
            
            Index getNumOutputChannels() const {
                return 1;
            }
            
            void processTempoEvent(MillisecondTime time, Tempo tempo) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->p_01->processTempoEvent(time, tempo);
                this->p_02->processTempoEvent(time, tempo);
                this->p_03->processTempoEvent(time, tempo);
                this->p_04->processTempoEvent(time, tempo);
            }
            
            void processTransportEvent(MillisecondTime time, TransportState state) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->p_01->processTransportEvent(time, state);
                this->p_02->processTransportEvent(time, state);
                this->p_03->processTransportEvent(time, state);
                this->p_04->processTransportEvent(time, state);
            }
            
            void processBeatTimeEvent(MillisecondTime time, BeatTime beattime) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->p_01->processBeatTimeEvent(time, beattime);
                this->p_02->processBeatTimeEvent(time, beattime);
                this->p_03->processBeatTimeEvent(time, beattime);
                this->p_04->processBeatTimeEvent(time, beattime);
                this->phasor_03_onBeatTimeChanged(beattime);
                this->phasor_04_onBeatTimeChanged(beattime);
            }
            
            void processTimeSignatureEvent(MillisecondTime time, Int numerator, Int denominator) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->p_01->processTimeSignatureEvent(time, numerator, denominator);
                this->p_02->processTimeSignatureEvent(time, numerator, denominator);
                this->p_03->processTimeSignatureEvent(time, numerator, denominator);
                this->p_04->processTimeSignatureEvent(time, numerator, denominator);
            }
            
            void processBBUEvent(MillisecondTime time, number bars, number beats, number units) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->p_01->processBBUEvent(time, bars, beats, units);
                this->p_02->processBBUEvent(time, bars, beats, units);
                this->p_03->processBBUEvent(time, bars, beats, units);
                this->p_04->processBBUEvent(time, bars, beats, units);
            }
            
            void getPreset(PatcherStateInterface& preset) {
                this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
                this->param_08_getPresetValue(getSubState(preset, "lfotype"));
                this->param_09_getPresetValue(getSubState(preset, "on"));
                this->param_10_getPresetValue(getSubState(preset, "jitter"));
                this->param_11_getPresetValue(getSubState(preset, "smooth"));
                this->param_12_getPresetValue(getSubState(preset, "pulsewidth"));
                this->p_01->getPreset(getSubState(getSubState(preset, "__sps"), "smooth[1]"));
                this->p_02->getPreset(getSubState(getSubState(preset, "__sps"), "jitter[1]"));
                this->p_03->getPreset(getSubState(getSubState(preset, "__sps"), "square"));
                this->p_04->getPreset(getSubState(getSubState(preset, "__sps"), "rect"));
            }
            
            void setPreset(MillisecondTime time, PatcherStateInterface& preset) {
                this->p_01->setPreset(time, getSubState(getSubState(preset, "__sps"), "smooth[1]"));
                this->p_02->setPreset(time, getSubState(getSubState(preset, "__sps"), "jitter[1]"));
                this->p_03->setPreset(time, getSubState(getSubState(preset, "__sps"), "square"));
                this->p_04->setPreset(time, getSubState(getSubState(preset, "__sps"), "rect"));
            }
            
            void setParameterValue(ParameterIndex index, ParameterValue v, MillisecondTime time) {
                this->updateTime(time, (ENGINE*)nullptr);
            
                switch (index) {
                case 0:
                    {
                    this->param_08_value_set(v);
                    break;
                    }
                case 1:
                    {
                    this->param_09_value_set(v);
                    break;
                    }
                case 2:
                    {
                    this->param_10_value_set(v);
                    break;
                    }
                case 3:
                    {
                    this->param_11_value_set(v);
                    break;
                    }
                case 4:
                    {
                    this->param_12_value_set(v);
                    break;
                    }
                default:
                    {
                    index -= 5;
            
                    if (index < this->p_01->getNumParameters())
                        this->p_01->setParameterValue(index, v, time);
            
                    index -= this->p_01->getNumParameters();
            
                    if (index < this->p_02->getNumParameters())
                        this->p_02->setParameterValue(index, v, time);
            
                    index -= this->p_02->getNumParameters();
            
                    if (index < this->p_03->getNumParameters())
                        this->p_03->setParameterValue(index, v, time);
            
                    index -= this->p_03->getNumParameters();
            
                    if (index < this->p_04->getNumParameters())
                        this->p_04->setParameterValue(index, v, time);
            
                    break;
                    }
                }
            }
            
            void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                this->setParameterValue(index, value, time);
            }
            
            void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
                this->setParameterValue(index, this->getParameterValue(index), time);
            }
            
            void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                this->setParameterValueNormalized(index, value, time);
            }
            
            ParameterValue getParameterValue(ParameterIndex index)  {
                switch (index) {
                case 0:
                    {
                    return this->param_08_value;
                    }
                case 1:
                    {
                    return this->param_09_value;
                    }
                case 2:
                    {
                    return this->param_10_value;
                    }
                case 3:
                    {
                    return this->param_11_value;
                    }
                case 4:
                    {
                    return this->param_12_value;
                    }
                default:
                    {
                    index -= 5;
            
                    if (index < this->p_01->getNumParameters())
                        return this->p_01->getParameterValue(index);
            
                    index -= this->p_01->getNumParameters();
            
                    if (index < this->p_02->getNumParameters())
                        return this->p_02->getParameterValue(index);
            
                    index -= this->p_02->getNumParameters();
            
                    if (index < this->p_03->getNumParameters())
                        return this->p_03->getParameterValue(index);
            
                    index -= this->p_03->getNumParameters();
            
                    if (index < this->p_04->getNumParameters())
                        return this->p_04->getParameterValue(index);
            
                    return 0;
                    }
                }
            }
            
            ParameterIndex getNumSignalInParameters() const {
                return 0;
            }
            
            ParameterIndex getNumSignalOutParameters() const {
                return 0;
            }
            
            ParameterIndex getNumParameters() const {
                return 5 + this->p_01->getNumParameters() + this->p_02->getNumParameters() + this->p_03->getNumParameters() + this->p_04->getNumParameters();
            }
            
            ConstCharPointer getParameterName(ParameterIndex index) const {
                switch (index) {
                case 0:
                    {
                    return "lfotype";
                    }
                case 1:
                    {
                    return "on";
                    }
                case 2:
                    {
                    return "jitter";
                    }
                case 3:
                    {
                    return "smooth";
                    }
                case 4:
                    {
                    return "pulsewidth";
                    }
                default:
                    {
                    index -= 5;
            
                    if (index < this->p_01->getNumParameters())
                        return this->p_01->getParameterName(index);
            
                    index -= this->p_01->getNumParameters();
            
                    if (index < this->p_02->getNumParameters())
                        return this->p_02->getParameterName(index);
            
                    index -= this->p_02->getNumParameters();
            
                    if (index < this->p_03->getNumParameters())
                        return this->p_03->getParameterName(index);
            
                    index -= this->p_03->getNumParameters();
            
                    if (index < this->p_04->getNumParameters())
                        return this->p_04->getParameterName(index);
            
                    return "bogus";
                    }
                }
            }
            
            ConstCharPointer getParameterId(ParameterIndex index) const {
                switch (index) {
                case 0:
                    {
                    return "poly/osc.analog[1]/lfotype";
                    }
                case 1:
                    {
                    return "poly/osc.analog[1]/on";
                    }
                case 2:
                    {
                    return "poly/osc.analog[1]/jitter";
                    }
                case 3:
                    {
                    return "poly/osc.analog[1]/smooth";
                    }
                case 4:
                    {
                    return "poly/osc.analog[1]/pulsewidth";
                    }
                default:
                    {
                    index -= 5;
            
                    if (index < this->p_01->getNumParameters())
                        return this->p_01->getParameterId(index);
            
                    index -= this->p_01->getNumParameters();
            
                    if (index < this->p_02->getNumParameters())
                        return this->p_02->getParameterId(index);
            
                    index -= this->p_02->getNumParameters();
            
                    if (index < this->p_03->getNumParameters())
                        return this->p_03->getParameterId(index);
            
                    index -= this->p_03->getNumParameters();
            
                    if (index < this->p_04->getNumParameters())
                        return this->p_04->getParameterId(index);
            
                    return "bogus";
                    }
                }
            }
            
            void getParameterInfo(ParameterIndex index, ParameterInfo * info) const {
                {
                    switch (index) {
                    case 0:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 0;
                        info->min = 0;
                        info->max = 4;
                        info->exponent = 1;
                        info->steps = 5;
                        static const char * eVal0[] = {"sine", "ramp", "square", "rect", "tri"};
                        info->enumValues = eVal0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    case 1:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 1;
                        info->min = 0;
                        info->max = 1;
                        info->exponent = 1;
                        info->steps = 2;
                        static const char * eVal1[] = {"false", "true"};
                        info->enumValues = eVal1;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    case 2:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 0;
                        info->min = 0;
                        info->max = 100;
                        info->exponent = 1;
                        info->steps = 0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    case 3:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 0;
                        info->min = 0;
                        info->max = 100;
                        info->exponent = 1;
                        info->steps = 0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    case 4:
                        {
                        info->type = ParameterTypeNumber;
                        info->initialValue = 0.8;
                        info->min = 0;
                        info->max = 1;
                        info->exponent = 1;
                        info->steps = 0;
                        info->debug = false;
                        info->saveable = true;
                        info->transmittable = true;
                        info->initialized = true;
                        info->visible = true;
                        info->displayName = "";
                        info->unit = "";
                        info->ioType = IOTypeUndefined;
                        info->signalIndex = INVALID_INDEX;
                        break;
                        }
                    default:
                        {
                        index -= 5;
            
                        if (index < this->p_01->getNumParameters())
                            this->p_01->getParameterInfo(index, info);
            
                        index -= this->p_01->getNumParameters();
            
                        if (index < this->p_02->getNumParameters())
                            this->p_02->getParameterInfo(index, info);
            
                        index -= this->p_02->getNumParameters();
            
                        if (index < this->p_03->getNumParameters())
                            this->p_03->getParameterInfo(index, info);
            
                        index -= this->p_03->getNumParameters();
            
                        if (index < this->p_04->getNumParameters())
                            this->p_04->getParameterInfo(index, info);
            
                        break;
                        }
                    }
                }
            }
            
            ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
                if (steps == 1) {
                    if (normalizedValue > 0) {
                        normalizedValue = 1.;
                    }
                } else {
                    ParameterValue oneStep = (number)1. / (steps - 1);
                    ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
                    normalizedValue = numberOfSteps * oneStep;
                }
            
                return normalizedValue;
            }
            
            ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                switch (index) {
                case 4:
                    {
                    {
                        value = (value < 0 ? 0 : (value > 1 ? 1 : value));
                        ParameterValue normalizedValue = (value - 0) / (1 - 0);
                        return normalizedValue;
                    }
                    }
                case 1:
                    {
                    {
                        value = (value < 0 ? 0 : (value > 1 ? 1 : value));
                        ParameterValue normalizedValue = (value - 0) / (1 - 0);
            
                        {
                            normalizedValue = this->applyStepsToNormalizedParameterValue(normalizedValue, 2);
                        }
            
                        return normalizedValue;
                    }
                    }
                case 0:
                    {
                    {
                        value = (value < 0 ? 0 : (value > 4 ? 4 : value));
                        ParameterValue normalizedValue = (value - 0) / (4 - 0);
            
                        {
                            normalizedValue = this->applyStepsToNormalizedParameterValue(normalizedValue, 5);
                        }
            
                        return normalizedValue;
                    }
                    }
                case 2:
                case 3:
                    {
                    {
                        value = (value < 0 ? 0 : (value > 100 ? 100 : value));
                        ParameterValue normalizedValue = (value - 0) / (100 - 0);
                        return normalizedValue;
                    }
                    }
                default:
                    {
                    index -= 5;
            
                    if (index < this->p_01->getNumParameters())
                        return this->p_01->convertToNormalizedParameterValue(index, value);
            
                    index -= this->p_01->getNumParameters();
            
                    if (index < this->p_02->getNumParameters())
                        return this->p_02->convertToNormalizedParameterValue(index, value);
            
                    index -= this->p_02->getNumParameters();
            
                    if (index < this->p_03->getNumParameters())
                        return this->p_03->convertToNormalizedParameterValue(index, value);
            
                    index -= this->p_03->getNumParameters();
            
                    if (index < this->p_04->getNumParameters())
                        return this->p_04->convertToNormalizedParameterValue(index, value);
            
                    return value;
                    }
                }
            }
            
            ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                value = (value < 0 ? 0 : (value > 1 ? 1 : value));
            
                switch (index) {
                case 4:
                    {
                    {
                        {
                            return 0 + value * (1 - 0);
                        }
                    }
                    }
                case 1:
                    {
                    {
                        {
                            value = this->applyStepsToNormalizedParameterValue(value, 2);
                        }
            
                        {
                            return 0 + value * (1 - 0);
                        }
                    }
                    }
                case 0:
                    {
                    {
                        {
                            value = this->applyStepsToNormalizedParameterValue(value, 5);
                        }
            
                        {
                            return 0 + value * (4 - 0);
                        }
                    }
                    }
                case 2:
                case 3:
                    {
                    {
                        {
                            return 0 + value * (100 - 0);
                        }
                    }
                    }
                default:
                    {
                    index -= 5;
            
                    if (index < this->p_01->getNumParameters())
                        return this->p_01->convertFromNormalizedParameterValue(index, value);
            
                    index -= this->p_01->getNumParameters();
            
                    if (index < this->p_02->getNumParameters())
                        return this->p_02->convertFromNormalizedParameterValue(index, value);
            
                    index -= this->p_02->getNumParameters();
            
                    if (index < this->p_03->getNumParameters())
                        return this->p_03->convertFromNormalizedParameterValue(index, value);
            
                    index -= this->p_03->getNumParameters();
            
                    if (index < this->p_04->getNumParameters())
                        return this->p_04->convertFromNormalizedParameterValue(index, value);
            
                    return value;
                    }
                }
            }
            
            ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
                switch (index) {
                case 0:
                    {
                    return this->param_08_value_constrain(value);
                    }
                case 1:
                    {
                    return this->param_09_value_constrain(value);
                    }
                case 2:
                    {
                    return this->param_10_value_constrain(value);
                    }
                case 3:
                    {
                    return this->param_11_value_constrain(value);
                    }
                case 4:
                    {
                    return this->param_12_value_constrain(value);
                    }
                default:
                    {
                    index -= 5;
            
                    if (index < this->p_01->getNumParameters())
                        return this->p_01->constrainParameterValue(index, value);
            
                    index -= this->p_01->getNumParameters();
            
                    if (index < this->p_02->getNumParameters())
                        return this->p_02->constrainParameterValue(index, value);
            
                    index -= this->p_02->getNumParameters();
            
                    if (index < this->p_03->getNumParameters())
                        return this->p_03->constrainParameterValue(index, value);
            
                    index -= this->p_03->getNumParameters();
            
                    if (index < this->p_04->getNumParameters())
                        return this->p_04->constrainParameterValue(index, value);
            
                    return value;
                    }
                }
            }
            
            void processNumMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, number payload) {
                RNBO_UNUSED(objectId);
                this->updateTime(time, (ENGINE*)nullptr);
                this->p_01->processNumMessage(tag, objectId, time, payload);
                this->p_02->processNumMessage(tag, objectId, time, payload);
                this->p_03->processNumMessage(tag, objectId, time, payload);
                this->p_04->processNumMessage(tag, objectId, time, payload);
            }
            
            void processListMessage(
                MessageTag tag,
                MessageTag objectId,
                MillisecondTime time,
                const list& payload
            ) {
                RNBO_UNUSED(objectId);
                this->updateTime(time, (ENGINE*)nullptr);
                this->p_01->processListMessage(tag, objectId, time, payload);
                this->p_02->processListMessage(tag, objectId, time, payload);
                this->p_03->processListMessage(tag, objectId, time, payload);
                this->p_04->processListMessage(tag, objectId, time, payload);
            }
            
            void processBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) {
                RNBO_UNUSED(objectId);
                this->updateTime(time, (ENGINE*)nullptr);
                this->p_01->processBangMessage(tag, objectId, time);
                this->p_02->processBangMessage(tag, objectId, time);
                this->p_03->processBangMessage(tag, objectId, time);
                this->p_04->processBangMessage(tag, objectId, time);
            }
            
            MessageTagInfo resolveTag(MessageTag tag) const {
                switch (tag) {
            
                }
            
                auto subpatchResult_0 = this->p_01->resolveTag(tag);
            
                if (subpatchResult_0)
                    return subpatchResult_0;
            
                auto subpatchResult_1 = this->p_02->resolveTag(tag);
            
                if (subpatchResult_1)
                    return subpatchResult_1;
            
                auto subpatchResult_2 = this->p_03->resolveTag(tag);
            
                if (subpatchResult_2)
                    return subpatchResult_2;
            
                auto subpatchResult_3 = this->p_04->resolveTag(tag);
            
                if (subpatchResult_3)
                    return subpatchResult_3;
            
                return nullptr;
            }
            
            DataRef* getDataRef(DataRefIndex index)  {
                switch (index) {
                default:
                    {
                    return nullptr;
                    }
                }
            }
            
            DataRefIndex getNumDataRefs() const {
                return 0;
            }
            
            void processDataViewUpdate(DataRefIndex index, MillisecondTime time) {
                this->updateTime(time, (ENGINE*)nullptr);
            
                if (index == 0) {
                    this->cycle_tilde_03_buffer = reInitDataView(
                        this->cycle_tilde_03_buffer,
                        this->getPatcher()->getPatcher()->RNBODefaultSinus
                    );
            
                    this->cycle_tilde_03_bufferUpdated();
                }
            
                this->p_01->processDataViewUpdate(index, time);
                this->p_02->processDataViewUpdate(index, time);
                this->p_03->processDataViewUpdate(index, time);
                this->p_04->processDataViewUpdate(index, time);
            }
            
            void initialize() {
                RNBO_ASSERT(!this->_isInitialized);
                this->assign_defaults();
                this->applyState();
                this->cycle_tilde_03_buffer = new SampleBuffer(this->getPatcher()->getPatcher()->RNBODefaultSinus);
                this->_isInitialized = true;
            }
            
            protected:
            
            class RNBOSubpatcher_945 : public PatcherInterfaceImpl {
                        
                        friend class RNBOSubpatcher_952;
                        friend class rnbomatic;
                        
                        public:
                        
                        RNBOSubpatcher_945()
                        {}
                        
                        ~RNBOSubpatcher_945()
                        {
                            deallocateSignals();
                        }
                        
                        Index getNumMidiInputPorts() const {
                            return 0;
                        }
                        
                        void processMidiEvent(MillisecondTime , int , ConstByteArray , Index ) {}
                        
                        Index getNumMidiOutputPorts() const {
                            return 0;
                        }
                        
                        void process(
                            const SampleValue * const* inputs,
                            Index numInputs,
                            SampleValue * const* outputs,
                            Index numOutputs,
                            Index n
                        ) {
                            this->vs = n;
                            this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
                            SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
                            const SampleValue * in1 = (numInputs >= 1 && inputs[0] ? inputs[0] : this->zeroBuffer);
                            const SampleValue * in2 = (numInputs >= 2 && inputs[1] ? inputs[1] : this->zeroBuffer);
                            this->mstosamps_tilde_01_perform(in2, this->signals[0], n);
                            this->slide_tilde_02_perform(in1, this->signals[0], this->signals[0], out1, n);
                            this->stackprotect_perform(n);
                            this->audioProcessSampleCount += this->vs;
                        }
                        
                        void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
                            RNBO_ASSERT(this->_isInitialized);
                        
                            if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
                                Index i;
                        
                                for (i = 0; i < 1; i++) {
                                    this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
                                }
                        
                                this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
                                this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
                                this->didAllocateSignals = true;
                            }
                        
                            const bool sampleRateChanged = sampleRate != this->sr;
                            const bool maxvsChanged = maxBlockSize != this->maxvs;
                            const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;
                        
                            if (sampleRateChanged || maxvsChanged) {
                                this->vs = maxBlockSize;
                                this->maxvs = maxBlockSize;
                                this->sr = sampleRate;
                                this->invsr = 1 / sampleRate;
                            }
                        
                            RNBO_UNUSED(forceDSPSetup);
                        
                            if (sampleRateChanged)
                                this->onSampleRateChanged(sampleRate);
                        }
                        
                        number msToSamps(MillisecondTime ms, number sampleRate) {
                            return ms * sampleRate * 0.001;
                        }
                        
                        MillisecondTime sampsToMs(SampleIndex samps) {
                            return samps * (this->invsr * 1000);
                        }
                        
                        Index getNumInputChannels() const {
                            return 2;
                        }
                        
                        Index getNumOutputChannels() const {
                            return 1;
                        }
                        
                        void getPreset(PatcherStateInterface& ) {}
                        
                        void setPreset(MillisecondTime , PatcherStateInterface& ) {}
                        
                        void setParameterValue(ParameterIndex , ParameterValue , MillisecondTime ) {}
                        
                        void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                            this->setParameterValue(index, value, time);
                        }
                        
                        void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
                            this->setParameterValue(index, this->getParameterValue(index), time);
                        }
                        
                        void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                            this->setParameterValueNormalized(index, value, time);
                        }
                        
                        ParameterValue getParameterValue(ParameterIndex index)  {
                            switch (index) {
                            default:
                                {
                                return 0;
                                }
                            }
                        }
                        
                        ParameterIndex getNumSignalInParameters() const {
                            return 0;
                        }
                        
                        ParameterIndex getNumSignalOutParameters() const {
                            return 0;
                        }
                        
                        ParameterIndex getNumParameters() const {
                            return 0;
                        }
                        
                        ConstCharPointer getParameterName(ParameterIndex index) const {
                            switch (index) {
                            default:
                                {
                                return "bogus";
                                }
                            }
                        }
                        
                        ConstCharPointer getParameterId(ParameterIndex index) const {
                            switch (index) {
                            default:
                                {
                                return "bogus";
                                }
                            }
                        }
                        
                        void getParameterInfo(ParameterIndex , ParameterInfo * ) const {}
                        
                        ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
                            if (steps == 1) {
                                if (normalizedValue > 0) {
                                    normalizedValue = 1.;
                                }
                            } else {
                                ParameterValue oneStep = (number)1. / (steps - 1);
                                ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
                                normalizedValue = numberOfSteps * oneStep;
                            }
                        
                            return normalizedValue;
                        }
                        
                        ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                            value = (value < 0 ? 0 : (value > 1 ? 1 : value));
                        
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        void processNumMessage(MessageTag , MessageTag , MillisecondTime , number ) {}
                        
                        void processListMessage(MessageTag , MessageTag , MillisecondTime , const list& ) {}
                        
                        void processBangMessage(MessageTag , MessageTag , MillisecondTime ) {}
                        
                        MessageTagInfo resolveTag(MessageTag tag) const {
                            switch (tag) {
                        
                            }
                        
                            return nullptr;
                        }
                        
                        DataRef* getDataRef(DataRefIndex index)  {
                            switch (index) {
                            default:
                                {
                                return nullptr;
                                }
                            }
                        }
                        
                        DataRefIndex getNumDataRefs() const {
                            return 0;
                        }
                        
                        void processDataViewUpdate(DataRefIndex , MillisecondTime ) {}
                        
                        void initialize() {
                            RNBO_ASSERT(!this->_isInitialized);
                            this->assign_defaults();
                            this->applyState();
                            this->_isInitialized = true;
                        }
                        
                        protected:
                        
                        void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
                        	if (time == TimeNow) time = getTopLevelPatcher()->getPatcherTime();
                        	getTopLevelPatcher()->processInternalEvents(time);
                        	updateTime(time, (EXTERNALENGINE*)nullptr);
                        }
                        
                        RNBOSubpatcher_945* operator->() {
                            return this;
                        }
                        const RNBOSubpatcher_945* operator->() const {
                            return this;
                        }
                        virtual RNBOSubpatcher_952* getPatcher() const {
                            return static_cast<RNBOSubpatcher_952 *>(_parentPatcher);
                        }
                        
                        rnbomatic* getTopLevelPatcher() {
                            return this->getPatcher()->getTopLevelPatcher();
                        }
                        
                        void cancelClockEvents()
                        {
                        }
                        
                        inline number safediv(number num, number denom) {
                            return (denom == 0.0 ? 0.0 : num / denom);
                        }
                        
                        number maximum(number x, number y) {
                            return (x < y ? y : x);
                        }
                        
                        MillisecondTime getPatcherTime() const {
                            return this->_currentTime;
                        }
                        
                        void deallocateSignals() {
                            Index i;
                        
                            for (i = 0; i < 1; i++) {
                                this->signals[i] = freeSignal(this->signals[i]);
                            }
                        
                            this->zeroBuffer = freeSignal(this->zeroBuffer);
                            this->dummyBuffer = freeSignal(this->dummyBuffer);
                        }
                        
                        Index getMaxBlockSize() const {
                            return this->maxvs;
                        }
                        
                        number getSampleRate() const {
                            return this->sr;
                        }
                        
                        bool hasFixedVectorSize() const {
                            return false;
                        }
                        
                        void setProbingTarget(MessageTag ) {}
                        
                        void initializeObjects() {}
                        
                        void setVoiceIndex(Index index)  {
                            this->_voiceIndex = index;
                        }
                        
                        void setNoteNumber(Int noteNumber)  {
                            this->_noteNumber = noteNumber;
                        }
                        
                        Index getIsMuted()  {
                            return this->isMuted;
                        }
                        
                        void setIsMuted(Index v)  {
                            this->isMuted = v;
                        }
                        
                        void onSampleRateChanged(double ) {}
                        
                        void extractState(PatcherStateInterface& ) {}
                        
                        void applyState() {}
                        
                        void setParameterOffset(ParameterIndex offset) {
                            this->parameterOffset = offset;
                        }
                        
                        void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}
                        
                        void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}
                        
                        void processOutletEvent(
                            EngineLink* sender,
                            OutletIndex index,
                            ParameterValue value,
                            MillisecondTime time
                        ) {
                            this->updateTime(time, (ENGINE*)nullptr);
                            this->processOutletAtCurrentTime(sender, index, value);
                        }
                        
                        void sendOutlet(OutletIndex index, ParameterValue value) {
                            this->getEngine()->sendOutlet(this, index, value);
                        }
                        
                        void startup() {}
                        
                        void fillDataRef(DataRefIndex , DataRef& ) {}
                        
                        void allocateDataRefs() {}
                        
                        void mstosamps_tilde_01_perform(const Sample * ms, SampleValue * out1, Index n) {
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                out1[(Index)i] = ms[(Index)i] * this->sr * 0.001;
                            }
                        }
                        
                        void slide_tilde_02_perform(
                            const Sample * x,
                            const Sample * up,
                            const Sample * down,
                            SampleValue * out1,
                            Index n
                        ) {
                            auto __slide_tilde_02_prev = this->slide_tilde_02_prev;
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                number temp = x[(Index)i] - __slide_tilde_02_prev;
                                auto iup = this->safediv(1., this->maximum(1., rnbo_abs(up[(Index)i])));
                                auto idown = this->safediv(1., this->maximum(1., rnbo_abs(down[(Index)i])));
                                __slide_tilde_02_prev = __slide_tilde_02_prev + ((x[(Index)i] > __slide_tilde_02_prev ? iup : idown)) * temp;
                                out1[(Index)i] = __slide_tilde_02_prev;
                            }
                        
                            this->slide_tilde_02_prev = __slide_tilde_02_prev;
                        }
                        
                        void stackprotect_perform(Index n) {
                            RNBO_UNUSED(n);
                            auto __stackprotect_count = this->stackprotect_count;
                            __stackprotect_count = 0;
                            this->stackprotect_count = __stackprotect_count;
                        }
                        
                        bool stackprotect_check() {
                            this->stackprotect_count++;
                        
                            if (this->stackprotect_count > 128) {
                                console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
                                return true;
                            }
                        
                            return false;
                        }
                        
                        Index getPatcherSerial() const {
                            return 0;
                        }
                        
                        void sendParameter(ParameterIndex index, bool ignoreValue) {
                            this->getPatcher()->sendParameter(index + this->parameterOffset, ignoreValue);
                        }
                        
                        void scheduleParamInit(ParameterIndex index, Index order) {
                            this->getPatcher()->scheduleParamInit(index + this->parameterOffset, order);
                        }
                        
                        void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
                            RNBO_UNUSED(inProcess);
                            RNBO_UNUSED(engine);
                            this->_currentTime = time;
                            auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));
                        
                            if (offset >= (SampleIndex)(this->vs))
                                offset = (SampleIndex)(this->vs) - 1;
                        
                            if (offset < 0)
                                offset = 0;
                        
                            this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
                        }
                        
                        void assign_defaults()
                        {
                            slide_tilde_02_x = 0;
                            slide_tilde_02_up = 44100;
                            slide_tilde_02_down = 44100;
                            mstosamps_tilde_01_ms = 0;
                            _currentTime = 0;
                            audioProcessSampleCount = 0;
                            sampleOffsetIntoNextAudioBuffer = 0;
                            zeroBuffer = nullptr;
                            dummyBuffer = nullptr;
                            signals[0] = nullptr;
                            didAllocateSignals = 0;
                            vs = 0;
                            maxvs = 0;
                            sr = 44100;
                            invsr = 0.000022675736961451248;
                            slide_tilde_02_prev = 0;
                            stackprotect_count = 0;
                            _voiceIndex = 0;
                            _noteNumber = 0;
                            isMuted = 1;
                            parameterOffset = 0;
                        }
                        
                        // member variables
                        
                            number slide_tilde_02_x;
                            number slide_tilde_02_up;
                            number slide_tilde_02_down;
                            number mstosamps_tilde_01_ms;
                            MillisecondTime _currentTime;
                            UInt64 audioProcessSampleCount;
                            Index sampleOffsetIntoNextAudioBuffer;
                            signal zeroBuffer;
                            signal dummyBuffer;
                            SampleValue * signals[1];
                            bool didAllocateSignals;
                            Index vs;
                            Index maxvs;
                            number sr;
                            number invsr;
                            number slide_tilde_02_prev;
                            number stackprotect_count;
                            Index _voiceIndex;
                            Int _noteNumber;
                            Index isMuted;
                            ParameterIndex parameterOffset;
                            bool _isInitialized = false;
            };
            
            class RNBOSubpatcher_946 : public PatcherInterfaceImpl {
                        
                        friend class RNBOSubpatcher_952;
                        friend class rnbomatic;
                        
                        public:
                        
                        RNBOSubpatcher_946()
                        {}
                        
                        ~RNBOSubpatcher_946()
                        {
                            deallocateSignals();
                        }
                        
                        Index getNumMidiInputPorts() const {
                            return 0;
                        }
                        
                        void processMidiEvent(MillisecondTime , int , ConstByteArray , Index ) {}
                        
                        Index getNumMidiOutputPorts() const {
                            return 0;
                        }
                        
                        void process(
                            const SampleValue * const* inputs,
                            Index numInputs,
                            SampleValue * const* outputs,
                            Index numOutputs,
                            Index n
                        ) {
                            this->vs = n;
                            this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
                            SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
                            const SampleValue * in1 = (numInputs >= 1 && inputs[0] ? inputs[0] : this->zeroBuffer);
                            const SampleValue * in2 = (numInputs >= 2 && inputs[1] ? inputs[1] : this->zeroBuffer);
                            this->dspexpr_12_perform(in2, this->dspexpr_12_in2, this->signals[0], n);
                            this->rand_tilde_01_perform(in2, this->signals[1], n);
                            this->dspexpr_11_perform(this->signals[1], this->signals[0], this->signals[2], n);
                            this->dspexpr_10_perform(in1, this->signals[2], out1, n);
                            this->stackprotect_perform(n);
                            this->audioProcessSampleCount += this->vs;
                        }
                        
                        void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
                            RNBO_ASSERT(this->_isInitialized);
                        
                            if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
                                Index i;
                        
                                for (i = 0; i < 3; i++) {
                                    this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
                                }
                        
                                this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
                                this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
                                this->didAllocateSignals = true;
                            }
                        
                            const bool sampleRateChanged = sampleRate != this->sr;
                            const bool maxvsChanged = maxBlockSize != this->maxvs;
                            const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;
                        
                            if (sampleRateChanged || maxvsChanged) {
                                this->vs = maxBlockSize;
                                this->maxvs = maxBlockSize;
                                this->sr = sampleRate;
                                this->invsr = 1 / sampleRate;
                            }
                        
                            this->rand_tilde_01_dspsetup(forceDSPSetup);
                        
                            if (sampleRateChanged)
                                this->onSampleRateChanged(sampleRate);
                        }
                        
                        number msToSamps(MillisecondTime ms, number sampleRate) {
                            return ms * sampleRate * 0.001;
                        }
                        
                        MillisecondTime sampsToMs(SampleIndex samps) {
                            return samps * (this->invsr * 1000);
                        }
                        
                        Index getNumInputChannels() const {
                            return 2;
                        }
                        
                        Index getNumOutputChannels() const {
                            return 1;
                        }
                        
                        void getPreset(PatcherStateInterface& ) {}
                        
                        void setPreset(MillisecondTime , PatcherStateInterface& ) {}
                        
                        void setParameterValue(ParameterIndex , ParameterValue , MillisecondTime ) {}
                        
                        void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                            this->setParameterValue(index, value, time);
                        }
                        
                        void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
                            this->setParameterValue(index, this->getParameterValue(index), time);
                        }
                        
                        void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                            this->setParameterValueNormalized(index, value, time);
                        }
                        
                        ParameterValue getParameterValue(ParameterIndex index)  {
                            switch (index) {
                            default:
                                {
                                return 0;
                                }
                            }
                        }
                        
                        ParameterIndex getNumSignalInParameters() const {
                            return 0;
                        }
                        
                        ParameterIndex getNumSignalOutParameters() const {
                            return 0;
                        }
                        
                        ParameterIndex getNumParameters() const {
                            return 0;
                        }
                        
                        ConstCharPointer getParameterName(ParameterIndex index) const {
                            switch (index) {
                            default:
                                {
                                return "bogus";
                                }
                            }
                        }
                        
                        ConstCharPointer getParameterId(ParameterIndex index) const {
                            switch (index) {
                            default:
                                {
                                return "bogus";
                                }
                            }
                        }
                        
                        void getParameterInfo(ParameterIndex , ParameterInfo * ) const {}
                        
                        ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
                            if (steps == 1) {
                                if (normalizedValue > 0) {
                                    normalizedValue = 1.;
                                }
                            } else {
                                ParameterValue oneStep = (number)1. / (steps - 1);
                                ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
                                normalizedValue = numberOfSteps * oneStep;
                            }
                        
                            return normalizedValue;
                        }
                        
                        ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                            value = (value < 0 ? 0 : (value > 1 ? 1 : value));
                        
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        void processNumMessage(MessageTag , MessageTag , MillisecondTime , number ) {}
                        
                        void processListMessage(MessageTag , MessageTag , MillisecondTime , const list& ) {}
                        
                        void processBangMessage(MessageTag , MessageTag , MillisecondTime ) {}
                        
                        MessageTagInfo resolveTag(MessageTag tag) const {
                            switch (tag) {
                        
                            }
                        
                            return nullptr;
                        }
                        
                        DataRef* getDataRef(DataRefIndex index)  {
                            switch (index) {
                            default:
                                {
                                return nullptr;
                                }
                            }
                        }
                        
                        DataRefIndex getNumDataRefs() const {
                            return 0;
                        }
                        
                        void processDataViewUpdate(DataRefIndex , MillisecondTime ) {}
                        
                        void initialize() {
                            RNBO_ASSERT(!this->_isInitialized);
                            this->assign_defaults();
                            this->applyState();
                            this->_isInitialized = true;
                        }
                        
                        protected:
                        
                        void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
                        	if (time == TimeNow) time = getTopLevelPatcher()->getPatcherTime();
                        	getTopLevelPatcher()->processInternalEvents(time);
                        	updateTime(time, (EXTERNALENGINE*)nullptr);
                        }
                        
                        RNBOSubpatcher_946* operator->() {
                            return this;
                        }
                        const RNBOSubpatcher_946* operator->() const {
                            return this;
                        }
                        virtual RNBOSubpatcher_952* getPatcher() const {
                            return static_cast<RNBOSubpatcher_952 *>(_parentPatcher);
                        }
                        
                        rnbomatic* getTopLevelPatcher() {
                            return this->getPatcher()->getTopLevelPatcher();
                        }
                        
                        void cancelClockEvents()
                        {
                        }
                        
                        Index voice() {
                            return this->_voiceIndex;
                        }
                        
                        number random(number low, number high) {
                            number range = high - low;
                            return globalrandom() * range + low;
                        }
                        
                        MillisecondTime getPatcherTime() const {
                            return this->_currentTime;
                        }
                        
                        void deallocateSignals() {
                            Index i;
                        
                            for (i = 0; i < 3; i++) {
                                this->signals[i] = freeSignal(this->signals[i]);
                            }
                        
                            this->zeroBuffer = freeSignal(this->zeroBuffer);
                            this->dummyBuffer = freeSignal(this->dummyBuffer);
                        }
                        
                        Index getMaxBlockSize() const {
                            return this->maxvs;
                        }
                        
                        number getSampleRate() const {
                            return this->sr;
                        }
                        
                        bool hasFixedVectorSize() const {
                            return false;
                        }
                        
                        void setProbingTarget(MessageTag ) {}
                        
                        void initializeObjects() {
                            this->rand_tilde_01_n_init();
                            this->rand_tilde_01_init();
                        }
                        
                        void setVoiceIndex(Index index)  {
                            this->_voiceIndex = index;
                        }
                        
                        void setNoteNumber(Int noteNumber)  {
                            this->_noteNumber = noteNumber;
                        }
                        
                        Index getIsMuted()  {
                            return this->isMuted;
                        }
                        
                        void setIsMuted(Index v)  {
                            this->isMuted = v;
                        }
                        
                        void onSampleRateChanged(double ) {}
                        
                        void extractState(PatcherStateInterface& ) {}
                        
                        void applyState() {}
                        
                        void setParameterOffset(ParameterIndex offset) {
                            this->parameterOffset = offset;
                        }
                        
                        void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}
                        
                        void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}
                        
                        void processOutletEvent(
                            EngineLink* sender,
                            OutletIndex index,
                            ParameterValue value,
                            MillisecondTime time
                        ) {
                            this->updateTime(time, (ENGINE*)nullptr);
                            this->processOutletAtCurrentTime(sender, index, value);
                        }
                        
                        void sendOutlet(OutletIndex index, ParameterValue value) {
                            this->getEngine()->sendOutlet(this, index, value);
                        }
                        
                        void startup() {}
                        
                        void fillDataRef(DataRefIndex , DataRef& ) {}
                        
                        void allocateDataRefs() {}
                        
                        void dspexpr_12_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
                            RNBO_UNUSED(in2);
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                out1[(Index)i] = in1[(Index)i] / (number)150;//#map:_###_obj_###_:1
                            }
                        }
                        
                        void rand_tilde_01_perform(const Sample * frequency, SampleValue * out1, Index n) {
                            auto __rand_tilde_01_lastValue = this->rand_tilde_01_lastValue;
                            auto __rand_tilde_01_oldTargetValue = this->rand_tilde_01_oldTargetValue;
                            auto __rand_tilde_01_targetValue = this->rand_tilde_01_targetValue;
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                number f = (frequency[(Index)i] > 0.0001 ? frequency[(Index)i] : 0.0001);
                                number phasorValue = this->rand_tilde_01_p_next(f, -1);
                        
                                if (this->rand_tilde_01_d_next(phasorValue) < 0) {
                                    __rand_tilde_01_targetValue = this->rand_tilde_01_n_next();
                                    __rand_tilde_01_oldTargetValue = __rand_tilde_01_lastValue;
                                }
                        
                                __rand_tilde_01_lastValue = __rand_tilde_01_oldTargetValue + phasorValue * (__rand_tilde_01_targetValue - __rand_tilde_01_oldTargetValue);
                                out1[(Index)i] = __rand_tilde_01_lastValue;
                            }
                        
                            this->rand_tilde_01_targetValue = __rand_tilde_01_targetValue;
                            this->rand_tilde_01_oldTargetValue = __rand_tilde_01_oldTargetValue;
                            this->rand_tilde_01_lastValue = __rand_tilde_01_lastValue;
                        }
                        
                        void dspexpr_11_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
                            }
                        }
                        
                        void dspexpr_10_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                out1[(Index)i] = in1[(Index)i] + in2[(Index)i];//#map:_###_obj_###_:1
                            }
                        }
                        
                        void stackprotect_perform(Index n) {
                            RNBO_UNUSED(n);
                            auto __stackprotect_count = this->stackprotect_count;
                            __stackprotect_count = 0;
                            this->stackprotect_count = __stackprotect_count;
                        }
                        
                        number rand_tilde_01_p_next(number freq, number reset) {
                            {
                                {
                                    if (reset >= 0.)
                                        this->rand_tilde_01_p_currentPhase = reset;
                                }
                            }
                        
                            number pincr = freq * this->rand_tilde_01_p_conv;
                        
                            if (this->rand_tilde_01_p_currentPhase < 0.)
                                this->rand_tilde_01_p_currentPhase = 1. + this->rand_tilde_01_p_currentPhase;
                        
                            if (this->rand_tilde_01_p_currentPhase > 1.)
                                this->rand_tilde_01_p_currentPhase = this->rand_tilde_01_p_currentPhase - 1.;
                        
                            number tmp = this->rand_tilde_01_p_currentPhase;
                            this->rand_tilde_01_p_currentPhase += pincr;
                            return tmp;
                        }
                        
                        void rand_tilde_01_p_reset() {
                            this->rand_tilde_01_p_currentPhase = 0;
                        }
                        
                        void rand_tilde_01_p_dspsetup() {
                            this->rand_tilde_01_p_conv = (number)1 / this->sr;
                        }
                        
                        void rand_tilde_01_n_reset() {
                            xoshiro_reset(
                                systemticks() + this->voice() + this->random(0, 10000),
                                this->rand_tilde_01_n_state
                            );
                        }
                        
                        void rand_tilde_01_n_init() {
                            this->rand_tilde_01_n_reset();
                        }
                        
                        void rand_tilde_01_n_seed(number v) {
                            xoshiro_reset(v, this->rand_tilde_01_n_state);
                        }
                        
                        number rand_tilde_01_n_next() {
                            return xoshiro_next(this->rand_tilde_01_n_state);
                        }
                        
                        number rand_tilde_01_d_next(number x) {
                            number temp = (number)(x - this->rand_tilde_01_d_prev);
                            this->rand_tilde_01_d_prev = x;
                            return temp;
                        }
                        
                        void rand_tilde_01_d_dspsetup() {
                            this->rand_tilde_01_d_reset();
                        }
                        
                        void rand_tilde_01_d_reset() {
                            this->rand_tilde_01_d_prev = 0;
                        }
                        
                        void rand_tilde_01_init() {
                            this->rand_tilde_01_targetValue = this->rand_tilde_01_n_next();
                        }
                        
                        void rand_tilde_01_dspsetup(bool force) {
                            if ((bool)(this->rand_tilde_01_setupDone) && (bool)(!(bool)(force)))
                                return;
                        
                            this->rand_tilde_01_setupDone = true;
                            this->rand_tilde_01_p_dspsetup();
                            this->rand_tilde_01_d_dspsetup();
                        }
                        
                        bool stackprotect_check() {
                            this->stackprotect_count++;
                        
                            if (this->stackprotect_count > 128) {
                                console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
                                return true;
                            }
                        
                            return false;
                        }
                        
                        Index getPatcherSerial() const {
                            return 0;
                        }
                        
                        void sendParameter(ParameterIndex index, bool ignoreValue) {
                            this->getPatcher()->sendParameter(index + this->parameterOffset, ignoreValue);
                        }
                        
                        void scheduleParamInit(ParameterIndex index, Index order) {
                            this->getPatcher()->scheduleParamInit(index + this->parameterOffset, order);
                        }
                        
                        void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
                            RNBO_UNUSED(inProcess);
                            RNBO_UNUSED(engine);
                            this->_currentTime = time;
                            auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));
                        
                            if (offset >= (SampleIndex)(this->vs))
                                offset = (SampleIndex)(this->vs) - 1;
                        
                            if (offset < 0)
                                offset = 0;
                        
                            this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
                        }
                        
                        void assign_defaults()
                        {
                            dspexpr_10_in1 = 0;
                            dspexpr_10_in2 = 0;
                            dspexpr_11_in1 = 0;
                            dspexpr_11_in2 = 0;
                            rand_tilde_01_frequency = 0;
                            dspexpr_12_in1 = 0;
                            dspexpr_12_in2 = 150;
                            _currentTime = 0;
                            audioProcessSampleCount = 0;
                            sampleOffsetIntoNextAudioBuffer = 0;
                            zeroBuffer = nullptr;
                            dummyBuffer = nullptr;
                            signals[0] = nullptr;
                            signals[1] = nullptr;
                            signals[2] = nullptr;
                            didAllocateSignals = 0;
                            vs = 0;
                            maxvs = 0;
                            sr = 44100;
                            invsr = 0.000022675736961451248;
                            rand_tilde_01_lastValue = 0;
                            rand_tilde_01_targetValue = 0;
                            rand_tilde_01_oldTargetValue = 0;
                            rand_tilde_01_p_currentPhase = 0;
                            rand_tilde_01_p_conv = 0;
                            rand_tilde_01_d_prev = 0;
                            rand_tilde_01_setupDone = false;
                            stackprotect_count = 0;
                            _voiceIndex = 0;
                            _noteNumber = 0;
                            isMuted = 1;
                            parameterOffset = 0;
                        }
                        
                        // member variables
                        
                            number dspexpr_10_in1;
                            number dspexpr_10_in2;
                            number dspexpr_11_in1;
                            number dspexpr_11_in2;
                            number rand_tilde_01_frequency;
                            number dspexpr_12_in1;
                            number dspexpr_12_in2;
                            MillisecondTime _currentTime;
                            UInt64 audioProcessSampleCount;
                            Index sampleOffsetIntoNextAudioBuffer;
                            signal zeroBuffer;
                            signal dummyBuffer;
                            SampleValue * signals[3];
                            bool didAllocateSignals;
                            Index vs;
                            Index maxvs;
                            number sr;
                            number invsr;
                            number rand_tilde_01_lastValue;
                            number rand_tilde_01_targetValue;
                            number rand_tilde_01_oldTargetValue;
                            number rand_tilde_01_p_currentPhase;
                            number rand_tilde_01_p_conv;
                            UInt rand_tilde_01_n_state[4] = { };
                            number rand_tilde_01_d_prev;
                            bool rand_tilde_01_setupDone;
                            number stackprotect_count;
                            Index _voiceIndex;
                            Int _noteNumber;
                            Index isMuted;
                            ParameterIndex parameterOffset;
                            bool _isInitialized = false;
            };
            
            class RNBOSubpatcher_947 : public PatcherInterfaceImpl {
                        
                        friend class RNBOSubpatcher_952;
                        friend class rnbomatic;
                        
                        public:
                        
                        RNBOSubpatcher_947()
                        {}
                        
                        ~RNBOSubpatcher_947()
                        {
                            deallocateSignals();
                        }
                        
                        Index getNumMidiInputPorts() const {
                            return 0;
                        }
                        
                        void processMidiEvent(MillisecondTime , int , ConstByteArray , Index ) {}
                        
                        Index getNumMidiOutputPorts() const {
                            return 0;
                        }
                        
                        void process(
                            const SampleValue * const* inputs,
                            Index numInputs,
                            SampleValue * const* outputs,
                            Index numOutputs,
                            Index n
                        ) {
                            this->vs = n;
                            this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
                            SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
                            const SampleValue * in1 = (numInputs >= 1 && inputs[0] ? inputs[0] : this->zeroBuffer);
                            this->phasor_01_perform(in1, this->signals[0], n);
                            this->dspexpr_14_perform(this->signals[0], this->dspexpr_14_in2, this->signals[1], n);
                            this->dspexpr_13_perform(this->signals[1], this->dspexpr_13_in2, this->signals[0], n);
                            this->ip_01_perform(this->signals[1], n);
                            this->ip_02_perform(this->signals[2], n);
                            this->selector_01_perform(this->signals[0], this->signals[1], this->signals[2], out1, n);
                            this->stackprotect_perform(n);
                            this->audioProcessSampleCount += this->vs;
                        }
                        
                        void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
                            RNBO_ASSERT(this->_isInitialized);
                        
                            if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
                                Index i;
                        
                                for (i = 0; i < 3; i++) {
                                    this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
                                }
                        
                                this->phasor_01_sigbuf = resizeSignal(this->phasor_01_sigbuf, this->maxvs, maxBlockSize);
                                this->ip_01_sigbuf = resizeSignal(this->ip_01_sigbuf, this->maxvs, maxBlockSize);
                                this->ip_02_sigbuf = resizeSignal(this->ip_02_sigbuf, this->maxvs, maxBlockSize);
                                this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
                                this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
                                this->didAllocateSignals = true;
                            }
                        
                            const bool sampleRateChanged = sampleRate != this->sr;
                            const bool maxvsChanged = maxBlockSize != this->maxvs;
                            const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;
                        
                            if (sampleRateChanged || maxvsChanged) {
                                this->vs = maxBlockSize;
                                this->maxvs = maxBlockSize;
                                this->sr = sampleRate;
                                this->invsr = 1 / sampleRate;
                            }
                        
                            this->phasor_01_dspsetup(forceDSPSetup);
                            this->ip_01_dspsetup(forceDSPSetup);
                            this->ip_02_dspsetup(forceDSPSetup);
                        
                            if (sampleRateChanged)
                                this->onSampleRateChanged(sampleRate);
                        }
                        
                        number msToSamps(MillisecondTime ms, number sampleRate) {
                            return ms * sampleRate * 0.001;
                        }
                        
                        MillisecondTime sampsToMs(SampleIndex samps) {
                            return samps * (this->invsr * 1000);
                        }
                        
                        Index getNumInputChannels() const {
                            return 1;
                        }
                        
                        Index getNumOutputChannels() const {
                            return 1;
                        }
                        
                        void processTempoEvent(MillisecondTime , Tempo ) {}
                        
                        void processTransportEvent(MillisecondTime , TransportState ) {}
                        
                        void processBeatTimeEvent(MillisecondTime time, BeatTime beattime) {
                            this->updateTime(time, (ENGINE*)nullptr);
                            this->phasor_01_onBeatTimeChanged(beattime);
                        }
                        
                        void processTimeSignatureEvent(MillisecondTime , Int , Int ) {}
                        
                        void processBBUEvent(MillisecondTime , number , number , number ) {}
                        
                        void getPreset(PatcherStateInterface& ) {}
                        
                        void setPreset(MillisecondTime , PatcherStateInterface& ) {}
                        
                        void setParameterValue(ParameterIndex , ParameterValue , MillisecondTime ) {}
                        
                        void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                            this->setParameterValue(index, value, time);
                        }
                        
                        void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
                            this->setParameterValue(index, this->getParameterValue(index), time);
                        }
                        
                        void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                            this->setParameterValueNormalized(index, value, time);
                        }
                        
                        ParameterValue getParameterValue(ParameterIndex index)  {
                            switch (index) {
                            default:
                                {
                                return 0;
                                }
                            }
                        }
                        
                        ParameterIndex getNumSignalInParameters() const {
                            return 0;
                        }
                        
                        ParameterIndex getNumSignalOutParameters() const {
                            return 0;
                        }
                        
                        ParameterIndex getNumParameters() const {
                            return 0;
                        }
                        
                        ConstCharPointer getParameterName(ParameterIndex index) const {
                            switch (index) {
                            default:
                                {
                                return "bogus";
                                }
                            }
                        }
                        
                        ConstCharPointer getParameterId(ParameterIndex index) const {
                            switch (index) {
                            default:
                                {
                                return "bogus";
                                }
                            }
                        }
                        
                        void getParameterInfo(ParameterIndex , ParameterInfo * ) const {}
                        
                        ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
                            if (steps == 1) {
                                if (normalizedValue > 0) {
                                    normalizedValue = 1.;
                                }
                            } else {
                                ParameterValue oneStep = (number)1. / (steps - 1);
                                ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
                                normalizedValue = numberOfSteps * oneStep;
                            }
                        
                            return normalizedValue;
                        }
                        
                        ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                            value = (value < 0 ? 0 : (value > 1 ? 1 : value));
                        
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        void processNumMessage(MessageTag , MessageTag , MillisecondTime , number ) {}
                        
                        void processListMessage(MessageTag , MessageTag , MillisecondTime , const list& ) {}
                        
                        void processBangMessage(MessageTag , MessageTag , MillisecondTime ) {}
                        
                        MessageTagInfo resolveTag(MessageTag tag) const {
                            switch (tag) {
                        
                            }
                        
                            return nullptr;
                        }
                        
                        DataRef* getDataRef(DataRefIndex index)  {
                            switch (index) {
                            default:
                                {
                                return nullptr;
                                }
                            }
                        }
                        
                        DataRefIndex getNumDataRefs() const {
                            return 0;
                        }
                        
                        void processDataViewUpdate(DataRefIndex , MillisecondTime ) {}
                        
                        void initialize() {
                            RNBO_ASSERT(!this->_isInitialized);
                            this->assign_defaults();
                            this->applyState();
                            this->_isInitialized = true;
                        }
                        
                        protected:
                        
                        void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
                        	if (time == TimeNow) time = getTopLevelPatcher()->getPatcherTime();
                        	getTopLevelPatcher()->processInternalEvents(time);
                        	updateTime(time, (EXTERNALENGINE*)nullptr);
                        }
                        
                        RNBOSubpatcher_947* operator->() {
                            return this;
                        }
                        const RNBOSubpatcher_947* operator->() const {
                            return this;
                        }
                        virtual RNBOSubpatcher_952* getPatcher() const {
                            return static_cast<RNBOSubpatcher_952 *>(_parentPatcher);
                        }
                        
                        rnbomatic* getTopLevelPatcher() {
                            return this->getPatcher()->getTopLevelPatcher();
                        }
                        
                        void cancelClockEvents()
                        {
                        }
                        
                        MillisecondTime getPatcherTime() const {
                            return this->_currentTime;
                        }
                        
                        void deallocateSignals() {
                            Index i;
                        
                            for (i = 0; i < 3; i++) {
                                this->signals[i] = freeSignal(this->signals[i]);
                            }
                        
                            this->phasor_01_sigbuf = freeSignal(this->phasor_01_sigbuf);
                            this->ip_01_sigbuf = freeSignal(this->ip_01_sigbuf);
                            this->ip_02_sigbuf = freeSignal(this->ip_02_sigbuf);
                            this->zeroBuffer = freeSignal(this->zeroBuffer);
                            this->dummyBuffer = freeSignal(this->dummyBuffer);
                        }
                        
                        Index getMaxBlockSize() const {
                            return this->maxvs;
                        }
                        
                        number getSampleRate() const {
                            return this->sr;
                        }
                        
                        bool hasFixedVectorSize() const {
                            return false;
                        }
                        
                        void setProbingTarget(MessageTag ) {}
                        
                        void initializeObjects() {
                            this->ip_01_init();
                            this->ip_02_init();
                        }
                        
                        void setVoiceIndex(Index index)  {
                            this->_voiceIndex = index;
                        }
                        
                        void setNoteNumber(Int noteNumber)  {
                            this->_noteNumber = noteNumber;
                        }
                        
                        Index getIsMuted()  {
                            return this->isMuted;
                        }
                        
                        void setIsMuted(Index v)  {
                            this->isMuted = v;
                        }
                        
                        void onSampleRateChanged(double ) {}
                        
                        void extractState(PatcherStateInterface& ) {}
                        
                        void applyState() {}
                        
                        void setParameterOffset(ParameterIndex offset) {
                            this->parameterOffset = offset;
                        }
                        
                        void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}
                        
                        void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}
                        
                        void processOutletEvent(
                            EngineLink* sender,
                            OutletIndex index,
                            ParameterValue value,
                            MillisecondTime time
                        ) {
                            this->updateTime(time, (ENGINE*)nullptr);
                            this->processOutletAtCurrentTime(sender, index, value);
                        }
                        
                        void sendOutlet(OutletIndex index, ParameterValue value) {
                            this->getEngine()->sendOutlet(this, index, value);
                        }
                        
                        void startup() {}
                        
                        void fillDataRef(DataRefIndex , DataRef& ) {}
                        
                        void allocateDataRefs() {}
                        
                        void phasor_01_perform(const Sample * freq, SampleValue * out, Index n) {
                            for (Index i = 0; i < n; i++) {
                                out[(Index)i] = this->phasor_01_ph_next(freq[(Index)i], -1);
                                this->phasor_01_sigbuf[(Index)i] = -1;
                            }
                        }
                        
                        void dspexpr_14_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
                            RNBO_UNUSED(in2);
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                out1[(Index)i] = in1[(Index)i] > 0.5;//#map:_###_obj_###_:1
                            }
                        }
                        
                        void dspexpr_13_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
                            RNBO_UNUSED(in2);
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                out1[(Index)i] = in1[(Index)i] + 1;//#map:_###_obj_###_:1
                            }
                        }
                        
                        void ip_01_perform(SampleValue * out, Index n) {
                            auto __ip_01_lastValue = this->ip_01_lastValue;
                            auto __ip_01_lastIndex = this->ip_01_lastIndex;
                        
                            for (Index i = 0; i < n; i++) {
                                out[(Index)i] = ((SampleIndex)(i) >= __ip_01_lastIndex ? __ip_01_lastValue : this->ip_01_sigbuf[(Index)i]);
                            }
                        
                            __ip_01_lastIndex = 0;
                            this->ip_01_lastIndex = __ip_01_lastIndex;
                        }
                        
                        void ip_02_perform(SampleValue * out, Index n) {
                            auto __ip_02_lastValue = this->ip_02_lastValue;
                            auto __ip_02_lastIndex = this->ip_02_lastIndex;
                        
                            for (Index i = 0; i < n; i++) {
                                out[(Index)i] = ((SampleIndex)(i) >= __ip_02_lastIndex ? __ip_02_lastValue : this->ip_02_sigbuf[(Index)i]);
                            }
                        
                            __ip_02_lastIndex = 0;
                            this->ip_02_lastIndex = __ip_02_lastIndex;
                        }
                        
                        void selector_01_perform(
                            const Sample * onoff,
                            const SampleValue * in1,
                            const SampleValue * in2,
                            SampleValue * out,
                            Index n
                        ) {
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                if (onoff[(Index)i] >= 1 && onoff[(Index)i] < 2)
                                    out[(Index)i] = in1[(Index)i];
                                else if (onoff[(Index)i] >= 2 && onoff[(Index)i] < 3)
                                    out[(Index)i] = in2[(Index)i];
                                else
                                    out[(Index)i] = 0;
                            }
                        }
                        
                        void stackprotect_perform(Index n) {
                            RNBO_UNUSED(n);
                            auto __stackprotect_count = this->stackprotect_count;
                            __stackprotect_count = 0;
                            this->stackprotect_count = __stackprotect_count;
                        }
                        
                        void phasor_01_onBeatTimeChanged(number beattime) {
                            RNBO_UNUSED(beattime);
                            this->phasor_01_recalcInc = true;
                            this->phasor_01_recalcPhase = true;
                        }
                        
                        number phasor_01_ph_next(number freq, number reset) {
                            RNBO_UNUSED(reset);
                            number pincr = freq * this->phasor_01_ph_conv;
                        
                            if (this->phasor_01_ph_currentPhase < 0.)
                                this->phasor_01_ph_currentPhase = 1. + this->phasor_01_ph_currentPhase;
                        
                            if (this->phasor_01_ph_currentPhase > 1.)
                                this->phasor_01_ph_currentPhase = this->phasor_01_ph_currentPhase - 1.;
                        
                            number tmp = this->phasor_01_ph_currentPhase;
                            this->phasor_01_ph_currentPhase += pincr;
                            return tmp;
                        }
                        
                        void phasor_01_ph_reset() {
                            this->phasor_01_ph_currentPhase = 0;
                        }
                        
                        void phasor_01_ph_dspsetup() {
                            this->phasor_01_ph_conv = (number)1 / this->sr;
                        }
                        
                        void phasor_01_dspsetup(bool force) {
                            if ((bool)(this->phasor_01_setupDone) && (bool)(!(bool)(force)))
                                return;
                        
                            this->phasor_01_setupDone = true;
                            this->phasor_01_ph_dspsetup();
                        }
                        
                        void ip_01_init() {
                            this->ip_01_lastValue = this->ip_01_value;
                        }
                        
                        void ip_01_dspsetup(bool force) {
                            if ((bool)(this->ip_01_setupDone) && (bool)(!(bool)(force)))
                                return;
                        
                            this->ip_01_lastIndex = 0;
                            this->ip_01_setupDone = true;
                        }
                        
                        void ip_02_init() {
                            this->ip_02_lastValue = this->ip_02_value;
                        }
                        
                        void ip_02_dspsetup(bool force) {
                            if ((bool)(this->ip_02_setupDone) && (bool)(!(bool)(force)))
                                return;
                        
                            this->ip_02_lastIndex = 0;
                            this->ip_02_setupDone = true;
                        }
                        
                        bool stackprotect_check() {
                            this->stackprotect_count++;
                        
                            if (this->stackprotect_count > 128) {
                                console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
                                return true;
                            }
                        
                            return false;
                        }
                        
                        Index getPatcherSerial() const {
                            return 0;
                        }
                        
                        void sendParameter(ParameterIndex index, bool ignoreValue) {
                            this->getPatcher()->sendParameter(index + this->parameterOffset, ignoreValue);
                        }
                        
                        void scheduleParamInit(ParameterIndex index, Index order) {
                            this->getPatcher()->scheduleParamInit(index + this->parameterOffset, order);
                        }
                        
                        void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
                            RNBO_UNUSED(inProcess);
                            RNBO_UNUSED(engine);
                            this->_currentTime = time;
                            auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));
                        
                            if (offset >= (SampleIndex)(this->vs))
                                offset = (SampleIndex)(this->vs) - 1;
                        
                            if (offset < 0)
                                offset = 0;
                        
                            this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
                        }
                        
                        void assign_defaults()
                        {
                            dspexpr_13_in1 = 0;
                            dspexpr_13_in2 = 1;
                            selector_01_onoff = 1;
                            dspexpr_14_in1 = 0;
                            dspexpr_14_in2 = 0.5;
                            phasor_01_freq = 0;
                            ip_01_value = 1;
                            ip_01_impulse = 0;
                            ip_02_value = 0;
                            ip_02_impulse = 0;
                            _currentTime = 0;
                            audioProcessSampleCount = 0;
                            sampleOffsetIntoNextAudioBuffer = 0;
                            zeroBuffer = nullptr;
                            dummyBuffer = nullptr;
                            signals[0] = nullptr;
                            signals[1] = nullptr;
                            signals[2] = nullptr;
                            didAllocateSignals = 0;
                            vs = 0;
                            maxvs = 0;
                            sr = 44100;
                            invsr = 0.000022675736961451248;
                            phasor_01_sigbuf = nullptr;
                            phasor_01_lastLockedPhase = 0;
                            phasor_01_lastQuantum = 0;
                            phasor_01_lastTempo = 0;
                            phasor_01_nextJumpInSamples = 0;
                            phasor_01_inc = 0;
                            phasor_01_recalcInc = true;
                            phasor_01_recalcPhase = true;
                            phasor_01_ph_currentPhase = 0;
                            phasor_01_ph_conv = 0;
                            phasor_01_setupDone = false;
                            ip_01_lastIndex = 0;
                            ip_01_lastValue = 0;
                            ip_01_resetCount = 0;
                            ip_01_sigbuf = nullptr;
                            ip_01_setupDone = false;
                            ip_02_lastIndex = 0;
                            ip_02_lastValue = 0;
                            ip_02_resetCount = 0;
                            ip_02_sigbuf = nullptr;
                            ip_02_setupDone = false;
                            stackprotect_count = 0;
                            _voiceIndex = 0;
                            _noteNumber = 0;
                            isMuted = 1;
                            parameterOffset = 0;
                        }
                        
                        // member variables
                        
                            number dspexpr_13_in1;
                            number dspexpr_13_in2;
                            number selector_01_onoff;
                            number dspexpr_14_in1;
                            number dspexpr_14_in2;
                            number phasor_01_freq;
                            number ip_01_value;
                            number ip_01_impulse;
                            number ip_02_value;
                            number ip_02_impulse;
                            MillisecondTime _currentTime;
                            UInt64 audioProcessSampleCount;
                            Index sampleOffsetIntoNextAudioBuffer;
                            signal zeroBuffer;
                            signal dummyBuffer;
                            SampleValue * signals[3];
                            bool didAllocateSignals;
                            Index vs;
                            Index maxvs;
                            number sr;
                            number invsr;
                            signal phasor_01_sigbuf;
                            number phasor_01_lastLockedPhase;
                            number phasor_01_lastQuantum;
                            number phasor_01_lastTempo;
                            number phasor_01_nextJumpInSamples;
                            number phasor_01_inc;
                            bool phasor_01_recalcInc;
                            bool phasor_01_recalcPhase;
                            number phasor_01_ph_currentPhase;
                            number phasor_01_ph_conv;
                            bool phasor_01_setupDone;
                            SampleIndex ip_01_lastIndex;
                            number ip_01_lastValue;
                            SampleIndex ip_01_resetCount;
                            signal ip_01_sigbuf;
                            bool ip_01_setupDone;
                            SampleIndex ip_02_lastIndex;
                            number ip_02_lastValue;
                            SampleIndex ip_02_resetCount;
                            signal ip_02_sigbuf;
                            bool ip_02_setupDone;
                            number stackprotect_count;
                            Index _voiceIndex;
                            Int _noteNumber;
                            Index isMuted;
                            ParameterIndex parameterOffset;
                            bool _isInitialized = false;
            };
            
            class RNBOSubpatcher_948 : public PatcherInterfaceImpl {
                        
                        friend class RNBOSubpatcher_952;
                        friend class rnbomatic;
                        
                        public:
                        
                        RNBOSubpatcher_948()
                        {}
                        
                        ~RNBOSubpatcher_948()
                        {
                            deallocateSignals();
                        }
                        
                        Index getNumMidiInputPorts() const {
                            return 0;
                        }
                        
                        void processMidiEvent(MillisecondTime , int , ConstByteArray , Index ) {}
                        
                        Index getNumMidiOutputPorts() const {
                            return 0;
                        }
                        
                        void process(
                            const SampleValue * const* inputs,
                            Index numInputs,
                            SampleValue * const* outputs,
                            Index numOutputs,
                            Index n
                        ) {
                            this->vs = n;
                            this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
                            SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
                            const SampleValue * in1 = (numInputs >= 1 && inputs[0] ? inputs[0] : this->zeroBuffer);
                            this->phasor_02_perform(in1, this->signals[0], n);
                            this->dspexpr_16_perform(this->signals[0], this->dspexpr_16_in2, this->signals[1], n);
                            this->dspexpr_15_perform(this->signals[1], this->dspexpr_15_in2, this->signals[0], n);
                            this->ip_03_perform(this->signals[1], n);
                            this->ip_04_perform(this->signals[2], n);
                            this->selector_02_perform(this->signals[0], this->signals[1], this->signals[2], out1, n);
                            this->stackprotect_perform(n);
                            this->audioProcessSampleCount += this->vs;
                        }
                        
                        void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
                            RNBO_ASSERT(this->_isInitialized);
                        
                            if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
                                Index i;
                        
                                for (i = 0; i < 3; i++) {
                                    this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
                                }
                        
                                this->phasor_02_sigbuf = resizeSignal(this->phasor_02_sigbuf, this->maxvs, maxBlockSize);
                                this->ip_03_sigbuf = resizeSignal(this->ip_03_sigbuf, this->maxvs, maxBlockSize);
                                this->ip_04_sigbuf = resizeSignal(this->ip_04_sigbuf, this->maxvs, maxBlockSize);
                                this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
                                this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
                                this->didAllocateSignals = true;
                            }
                        
                            const bool sampleRateChanged = sampleRate != this->sr;
                            const bool maxvsChanged = maxBlockSize != this->maxvs;
                            const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;
                        
                            if (sampleRateChanged || maxvsChanged) {
                                this->vs = maxBlockSize;
                                this->maxvs = maxBlockSize;
                                this->sr = sampleRate;
                                this->invsr = 1 / sampleRate;
                            }
                        
                            this->phasor_02_dspsetup(forceDSPSetup);
                            this->ip_03_dspsetup(forceDSPSetup);
                            this->ip_04_dspsetup(forceDSPSetup);
                        
                            if (sampleRateChanged)
                                this->onSampleRateChanged(sampleRate);
                        }
                        
                        number msToSamps(MillisecondTime ms, number sampleRate) {
                            return ms * sampleRate * 0.001;
                        }
                        
                        MillisecondTime sampsToMs(SampleIndex samps) {
                            return samps * (this->invsr * 1000);
                        }
                        
                        Index getNumInputChannels() const {
                            return 1;
                        }
                        
                        Index getNumOutputChannels() const {
                            return 1;
                        }
                        
                        void processTempoEvent(MillisecondTime , Tempo ) {}
                        
                        void processTransportEvent(MillisecondTime , TransportState ) {}
                        
                        void processBeatTimeEvent(MillisecondTime time, BeatTime beattime) {
                            this->updateTime(time, (ENGINE*)nullptr);
                            this->phasor_02_onBeatTimeChanged(beattime);
                        }
                        
                        void processTimeSignatureEvent(MillisecondTime , Int , Int ) {}
                        
                        void processBBUEvent(MillisecondTime , number , number , number ) {}
                        
                        void getPreset(PatcherStateInterface& ) {}
                        
                        void setPreset(MillisecondTime , PatcherStateInterface& ) {}
                        
                        void setParameterValue(ParameterIndex , ParameterValue , MillisecondTime ) {}
                        
                        void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                            this->setParameterValue(index, value, time);
                        }
                        
                        void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
                            this->setParameterValue(index, this->getParameterValue(index), time);
                        }
                        
                        void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
                            this->setParameterValueNormalized(index, value, time);
                        }
                        
                        ParameterValue getParameterValue(ParameterIndex index)  {
                            switch (index) {
                            default:
                                {
                                return 0;
                                }
                            }
                        }
                        
                        ParameterIndex getNumSignalInParameters() const {
                            return 0;
                        }
                        
                        ParameterIndex getNumSignalOutParameters() const {
                            return 0;
                        }
                        
                        ParameterIndex getNumParameters() const {
                            return 0;
                        }
                        
                        ConstCharPointer getParameterName(ParameterIndex index) const {
                            switch (index) {
                            default:
                                {
                                return "bogus";
                                }
                            }
                        }
                        
                        ConstCharPointer getParameterId(ParameterIndex index) const {
                            switch (index) {
                            default:
                                {
                                return "bogus";
                                }
                            }
                        }
                        
                        void getParameterInfo(ParameterIndex , ParameterInfo * ) const {}
                        
                        ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
                            if (steps == 1) {
                                if (normalizedValue > 0) {
                                    normalizedValue = 1.;
                                }
                            } else {
                                ParameterValue oneStep = (number)1. / (steps - 1);
                                ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
                                normalizedValue = numberOfSteps * oneStep;
                            }
                        
                            return normalizedValue;
                        }
                        
                        ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
                            value = (value < 0 ? 0 : (value > 1 ? 1 : value));
                        
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
                            switch (index) {
                            default:
                                {
                                return value;
                                }
                            }
                        }
                        
                        void processNumMessage(MessageTag , MessageTag , MillisecondTime , number ) {}
                        
                        void processListMessage(MessageTag , MessageTag , MillisecondTime , const list& ) {}
                        
                        void processBangMessage(MessageTag , MessageTag , MillisecondTime ) {}
                        
                        MessageTagInfo resolveTag(MessageTag tag) const {
                            switch (tag) {
                        
                            }
                        
                            return nullptr;
                        }
                        
                        DataRef* getDataRef(DataRefIndex index)  {
                            switch (index) {
                            default:
                                {
                                return nullptr;
                                }
                            }
                        }
                        
                        DataRefIndex getNumDataRefs() const {
                            return 0;
                        }
                        
                        void processDataViewUpdate(DataRefIndex , MillisecondTime ) {}
                        
                        void initialize() {
                            RNBO_ASSERT(!this->_isInitialized);
                            this->assign_defaults();
                            this->applyState();
                            this->_isInitialized = true;
                        }
                        
                        protected:
                        
                        void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
                        	if (time == TimeNow) time = getTopLevelPatcher()->getPatcherTime();
                        	getTopLevelPatcher()->processInternalEvents(time);
                        	updateTime(time, (EXTERNALENGINE*)nullptr);
                        }
                        
                        RNBOSubpatcher_948* operator->() {
                            return this;
                        }
                        const RNBOSubpatcher_948* operator->() const {
                            return this;
                        }
                        virtual RNBOSubpatcher_952* getPatcher() const {
                            return static_cast<RNBOSubpatcher_952 *>(_parentPatcher);
                        }
                        
                        rnbomatic* getTopLevelPatcher() {
                            return this->getPatcher()->getTopLevelPatcher();
                        }
                        
                        void cancelClockEvents()
                        {
                        }
                        
                        MillisecondTime getPatcherTime() const {
                            return this->_currentTime;
                        }
                        
                        void eventinlet_01_out1_bang_bang() {}
                        
                        template<typename LISTTYPE> void eventinlet_01_out1_list_set(const LISTTYPE& v) {
                            {
                                number converted = (v->length > 0 ? v[0] : 0);
                                this->dspexpr_16_in2_set(converted);
                            }
                        }
                        
                        void deallocateSignals() {
                            Index i;
                        
                            for (i = 0; i < 3; i++) {
                                this->signals[i] = freeSignal(this->signals[i]);
                            }
                        
                            this->phasor_02_sigbuf = freeSignal(this->phasor_02_sigbuf);
                            this->ip_03_sigbuf = freeSignal(this->ip_03_sigbuf);
                            this->ip_04_sigbuf = freeSignal(this->ip_04_sigbuf);
                            this->zeroBuffer = freeSignal(this->zeroBuffer);
                            this->dummyBuffer = freeSignal(this->dummyBuffer);
                        }
                        
                        Index getMaxBlockSize() const {
                            return this->maxvs;
                        }
                        
                        number getSampleRate() const {
                            return this->sr;
                        }
                        
                        bool hasFixedVectorSize() const {
                            return false;
                        }
                        
                        void setProbingTarget(MessageTag ) {}
                        
                        void initializeObjects() {
                            this->ip_03_init();
                            this->ip_04_init();
                        }
                        
                        void setVoiceIndex(Index index)  {
                            this->_voiceIndex = index;
                        }
                        
                        void setNoteNumber(Int noteNumber)  {
                            this->_noteNumber = noteNumber;
                        }
                        
                        Index getIsMuted()  {
                            return this->isMuted;
                        }
                        
                        void setIsMuted(Index v)  {
                            this->isMuted = v;
                        }
                        
                        void onSampleRateChanged(double ) {}
                        
                        void extractState(PatcherStateInterface& ) {}
                        
                        void applyState() {}
                        
                        void setParameterOffset(ParameterIndex offset) {
                            this->parameterOffset = offset;
                        }
                        
                        void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}
                        
                        void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}
                        
                        void processOutletEvent(
                            EngineLink* sender,
                            OutletIndex index,
                            ParameterValue value,
                            MillisecondTime time
                        ) {
                            this->updateTime(time, (ENGINE*)nullptr);
                            this->processOutletAtCurrentTime(sender, index, value);
                        }
                        
                        void sendOutlet(OutletIndex index, ParameterValue value) {
                            this->getEngine()->sendOutlet(this, index, value);
                        }
                        
                        void startup() {}
                        
                        void fillDataRef(DataRefIndex , DataRef& ) {}
                        
                        void allocateDataRefs() {}
                        
                        void dspexpr_16_in2_set(number v) {
                            this->dspexpr_16_in2 = v;
                        }
                        
                        void eventinlet_01_out1_number_set(number v) {
                            this->dspexpr_16_in2_set(v);
                        }
                        
                        void phasor_02_perform(const Sample * freq, SampleValue * out, Index n) {
                            for (Index i = 0; i < n; i++) {
                                out[(Index)i] = this->phasor_02_ph_next(freq[(Index)i], -1);
                                this->phasor_02_sigbuf[(Index)i] = -1;
                            }
                        }
                        
                        void dspexpr_16_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                out1[(Index)i] = in1[(Index)i] > in2;//#map:_###_obj_###_:1
                            }
                        }
                        
                        void dspexpr_15_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
                            RNBO_UNUSED(in2);
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                out1[(Index)i] = in1[(Index)i] + 1;//#map:_###_obj_###_:1
                            }
                        }
                        
                        void ip_03_perform(SampleValue * out, Index n) {
                            auto __ip_03_lastValue = this->ip_03_lastValue;
                            auto __ip_03_lastIndex = this->ip_03_lastIndex;
                        
                            for (Index i = 0; i < n; i++) {
                                out[(Index)i] = ((SampleIndex)(i) >= __ip_03_lastIndex ? __ip_03_lastValue : this->ip_03_sigbuf[(Index)i]);
                            }
                        
                            __ip_03_lastIndex = 0;
                            this->ip_03_lastIndex = __ip_03_lastIndex;
                        }
                        
                        void ip_04_perform(SampleValue * out, Index n) {
                            auto __ip_04_lastValue = this->ip_04_lastValue;
                            auto __ip_04_lastIndex = this->ip_04_lastIndex;
                        
                            for (Index i = 0; i < n; i++) {
                                out[(Index)i] = ((SampleIndex)(i) >= __ip_04_lastIndex ? __ip_04_lastValue : this->ip_04_sigbuf[(Index)i]);
                            }
                        
                            __ip_04_lastIndex = 0;
                            this->ip_04_lastIndex = __ip_04_lastIndex;
                        }
                        
                        void selector_02_perform(
                            const Sample * onoff,
                            const SampleValue * in1,
                            const SampleValue * in2,
                            SampleValue * out,
                            Index n
                        ) {
                            Index i;
                        
                            for (i = 0; i < (Index)n; i++) {
                                if (onoff[(Index)i] >= 1 && onoff[(Index)i] < 2)
                                    out[(Index)i] = in1[(Index)i];
                                else if (onoff[(Index)i] >= 2 && onoff[(Index)i] < 3)
                                    out[(Index)i] = in2[(Index)i];
                                else
                                    out[(Index)i] = 0;
                            }
                        }
                        
                        void stackprotect_perform(Index n) {
                            RNBO_UNUSED(n);
                            auto __stackprotect_count = this->stackprotect_count;
                            __stackprotect_count = 0;
                            this->stackprotect_count = __stackprotect_count;
                        }
                        
                        void phasor_02_onBeatTimeChanged(number beattime) {
                            RNBO_UNUSED(beattime);
                            this->phasor_02_recalcInc = true;
                            this->phasor_02_recalcPhase = true;
                        }
                        
                        number phasor_02_ph_next(number freq, number reset) {
                            RNBO_UNUSED(reset);
                            number pincr = freq * this->phasor_02_ph_conv;
                        
                            if (this->phasor_02_ph_currentPhase < 0.)
                                this->phasor_02_ph_currentPhase = 1. + this->phasor_02_ph_currentPhase;
                        
                            if (this->phasor_02_ph_currentPhase > 1.)
                                this->phasor_02_ph_currentPhase = this->phasor_02_ph_currentPhase - 1.;
                        
                            number tmp = this->phasor_02_ph_currentPhase;
                            this->phasor_02_ph_currentPhase += pincr;
                            return tmp;
                        }
                        
                        void phasor_02_ph_reset() {
                            this->phasor_02_ph_currentPhase = 0;
                        }
                        
                        void phasor_02_ph_dspsetup() {
                            this->phasor_02_ph_conv = (number)1 / this->sr;
                        }
                        
                        void phasor_02_dspsetup(bool force) {
                            if ((bool)(this->phasor_02_setupDone) && (bool)(!(bool)(force)))
                                return;
                        
                            this->phasor_02_setupDone = true;
                            this->phasor_02_ph_dspsetup();
                        }
                        
                        void ip_03_init() {
                            this->ip_03_lastValue = this->ip_03_value;
                        }
                        
                        void ip_03_dspsetup(bool force) {
                            if ((bool)(this->ip_03_setupDone) && (bool)(!(bool)(force)))
                                return;
                        
                            this->ip_03_lastIndex = 0;
                            this->ip_03_setupDone = true;
                        }
                        
                        void ip_04_init() {
                            this->ip_04_lastValue = this->ip_04_value;
                        }
                        
                        void ip_04_dspsetup(bool force) {
                            if ((bool)(this->ip_04_setupDone) && (bool)(!(bool)(force)))
                                return;
                        
                            this->ip_04_lastIndex = 0;
                            this->ip_04_setupDone = true;
                        }
                        
                        bool stackprotect_check() {
                            this->stackprotect_count++;
                        
                            if (this->stackprotect_count > 128) {
                                console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
                                return true;
                            }
                        
                            return false;
                        }
                        
                        Index getPatcherSerial() const {
                            return 0;
                        }
                        
                        void sendParameter(ParameterIndex index, bool ignoreValue) {
                            this->getPatcher()->sendParameter(index + this->parameterOffset, ignoreValue);
                        }
                        
                        void scheduleParamInit(ParameterIndex index, Index order) {
                            this->getPatcher()->scheduleParamInit(index + this->parameterOffset, order);
                        }
                        
                        void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
                            RNBO_UNUSED(inProcess);
                            RNBO_UNUSED(engine);
                            this->_currentTime = time;
                            auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));
                        
                            if (offset >= (SampleIndex)(this->vs))
                                offset = (SampleIndex)(this->vs) - 1;
                        
                            if (offset < 0)
                                offset = 0;
                        
                            this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
                        }
                        
                        void assign_defaults()
                        {
                            phasor_02_freq = 0;
                            dspexpr_15_in1 = 0;
                            dspexpr_15_in2 = 1;
                            selector_02_onoff = 1;
                            dspexpr_16_in1 = 0;
                            dspexpr_16_in2 = 0.5;
                            ip_03_value = 1;
                            ip_03_impulse = 0;
                            ip_04_value = 0;
                            ip_04_impulse = 0;
                            _currentTime = 0;
                            audioProcessSampleCount = 0;
                            sampleOffsetIntoNextAudioBuffer = 0;
                            zeroBuffer = nullptr;
                            dummyBuffer = nullptr;
                            signals[0] = nullptr;
                            signals[1] = nullptr;
                            signals[2] = nullptr;
                            didAllocateSignals = 0;
                            vs = 0;
                            maxvs = 0;
                            sr = 44100;
                            invsr = 0.000022675736961451248;
                            phasor_02_sigbuf = nullptr;
                            phasor_02_lastLockedPhase = 0;
                            phasor_02_lastQuantum = 0;
                            phasor_02_lastTempo = 0;
                            phasor_02_nextJumpInSamples = 0;
                            phasor_02_inc = 0;
                            phasor_02_recalcInc = true;
                            phasor_02_recalcPhase = true;
                            phasor_02_ph_currentPhase = 0;
                            phasor_02_ph_conv = 0;
                            phasor_02_setupDone = false;
                            ip_03_lastIndex = 0;
                            ip_03_lastValue = 0;
                            ip_03_resetCount = 0;
                            ip_03_sigbuf = nullptr;
                            ip_03_setupDone = false;
                            ip_04_lastIndex = 0;
                            ip_04_lastValue = 0;
                            ip_04_resetCount = 0;
                            ip_04_sigbuf = nullptr;
                            ip_04_setupDone = false;
                            stackprotect_count = 0;
                            _voiceIndex = 0;
                            _noteNumber = 0;
                            isMuted = 1;
                            parameterOffset = 0;
                        }
                        
                        // member variables
                        
                            number phasor_02_freq;
                            number dspexpr_15_in1;
                            number dspexpr_15_in2;
                            number selector_02_onoff;
                            number dspexpr_16_in1;
                            number dspexpr_16_in2;
                            number ip_03_value;
                            number ip_03_impulse;
                            number ip_04_value;
                            number ip_04_impulse;
                            MillisecondTime _currentTime;
                            UInt64 audioProcessSampleCount;
                            Index sampleOffsetIntoNextAudioBuffer;
                            signal zeroBuffer;
                            signal dummyBuffer;
                            SampleValue * signals[3];
                            bool didAllocateSignals;
                            Index vs;
                            Index maxvs;
                            number sr;
                            number invsr;
                            signal phasor_02_sigbuf;
                            number phasor_02_lastLockedPhase;
                            number phasor_02_lastQuantum;
                            number phasor_02_lastTempo;
                            number phasor_02_nextJumpInSamples;
                            number phasor_02_inc;
                            bool phasor_02_recalcInc;
                            bool phasor_02_recalcPhase;
                            number phasor_02_ph_currentPhase;
                            number phasor_02_ph_conv;
                            bool phasor_02_setupDone;
                            SampleIndex ip_03_lastIndex;
                            number ip_03_lastValue;
                            SampleIndex ip_03_resetCount;
                            signal ip_03_sigbuf;
                            bool ip_03_setupDone;
                            SampleIndex ip_04_lastIndex;
                            number ip_04_lastValue;
                            SampleIndex ip_04_resetCount;
                            signal ip_04_sigbuf;
                            bool ip_04_setupDone;
                            number stackprotect_count;
                            Index _voiceIndex;
                            Int _noteNumber;
                            Index isMuted;
                            ParameterIndex parameterOffset;
                            bool _isInitialized = false;
            };
            
            void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
            	if (time == TimeNow) time = getTopLevelPatcher()->getPatcherTime();
            	getTopLevelPatcher()->processInternalEvents(time);
            	updateTime(time, (EXTERNALENGINE*)nullptr);
            }
            
            RNBOSubpatcher_952* operator->() {
                return this;
            }
            const RNBOSubpatcher_952* operator->() const {
                return this;
            }
            virtual RNBOSubpatcher_953* getPatcher() const {
                return static_cast<RNBOSubpatcher_953 *>(_parentPatcher);
            }
            
            rnbomatic* getTopLevelPatcher() {
                return this->getPatcher()->getTopLevelPatcher();
            }
            
            void cancelClockEvents()
            {
            }
            
            inline number safediv(number num, number denom) {
                return (denom == 0.0 ? 0.0 : num / denom);
            }
            
            number safepow(number base, number exponent) {
                return fixnan(rnbo_pow(base, exponent));
            }
            
            number wrap(number x, number low, number high) {
                number lo;
                number hi;
            
                if (low == high)
                    return low;
            
                if (low > high) {
                    hi = low;
                    lo = high;
                } else {
                    lo = low;
                    hi = high;
                }
            
                number range = hi - lo;
            
                if (x >= lo && x < hi)
                    return x;
            
                if (range <= 0.000000001)
                    return lo;
            
                Int numWraps = (Int)(trunc((x - lo) / range));
                numWraps = numWraps - ((x < lo ? 1 : 0));
                number result = x - range * numWraps;
            
                if (result >= hi)
                    return result - range;
                else
                    return result;
            }
            
            number fromnormalized(Index index, number normalizedValue) {
                return this->convertFromNormalizedParameterValue(index, normalizedValue);
            }
            
            void param_08_value_set(number v) {
                v = this->param_08_value_constrain(v);
                this->param_08_value = v;
                this->sendParameter(0, false);
            
                if (this->param_08_value != this->param_08_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_08_lastValue = this->param_08_value;
                }
            
                this->expr_08_in1_set(v);
            }
            
            void param_09_value_set(number v) {
                v = this->param_09_value_constrain(v);
                this->param_09_value = v;
                this->sendParameter(1, false);
            
                if (this->param_09_value != this->param_09_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_09_lastValue = this->param_09_value;
                }
            
                this->ip_05_value_set(v);
            }
            
            void param_10_value_set(number v) {
                v = this->param_10_value_constrain(v);
                this->param_10_value = v;
                this->sendParameter(2, false);
            
                if (this->param_10_value != this->param_10_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_10_lastValue = this->param_10_value;
                }
            
                this->ip_06_value_set(v);
            }
            
            void param_11_value_set(number v) {
                v = this->param_11_value_constrain(v);
                this->param_11_value = v;
                this->sendParameter(3, false);
            
                if (this->param_11_value != this->param_11_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_11_lastValue = this->param_11_value;
                }
            
                this->ip_07_value_set(v);
            }
            
            void param_12_value_set(number v) {
                v = this->param_12_value_constrain(v);
                this->param_12_value = v;
                this->sendParameter(4, false);
            
                if (this->param_12_value != this->param_12_lastValue) {
                    this->getEngine()->presetTouched();
                    this->param_12_lastValue = this->param_12_value;
                }
            
                this->p_04_in2_number_set(v);
            }
            
            MillisecondTime getPatcherTime() const {
                return this->_currentTime;
            }
            
            void deallocateSignals() {
                Index i;
            
                for (i = 0; i < 9; i++) {
                    this->signals[i] = freeSignal(this->signals[i]);
                }
            
                this->ip_05_sigbuf = freeSignal(this->ip_05_sigbuf);
                this->ip_06_sigbuf = freeSignal(this->ip_06_sigbuf);
                this->ip_07_sigbuf = freeSignal(this->ip_07_sigbuf);
                this->phasor_03_sigbuf = freeSignal(this->phasor_03_sigbuf);
                this->phasor_04_sigbuf = freeSignal(this->phasor_04_sigbuf);
                this->zeroBuffer = freeSignal(this->zeroBuffer);
                this->dummyBuffer = freeSignal(this->dummyBuffer);
            }
            
            Index getMaxBlockSize() const {
                return this->maxvs;
            }
            
            number getSampleRate() const {
                return this->sr;
            }
            
            bool hasFixedVectorSize() const {
                return false;
            }
            
            void setProbingTarget(MessageTag ) {}
            
            void initializeObjects() {
                this->ip_05_init();
                this->ip_06_init();
                this->ip_07_init();
                this->p_01->initializeObjects();
                this->p_02->initializeObjects();
                this->p_03->initializeObjects();
                this->p_04->initializeObjects();
            }
            
            void setVoiceIndex(Index index)  {
                this->_voiceIndex = index;
                this->p_01->setVoiceIndex(index);
                this->p_02->setVoiceIndex(index);
                this->p_03->setVoiceIndex(index);
                this->p_04->setVoiceIndex(index);
            }
            
            void setNoteNumber(Int noteNumber)  {
                this->_noteNumber = noteNumber;
                this->p_01->setNoteNumber(noteNumber);
                this->p_02->setNoteNumber(noteNumber);
                this->p_03->setNoteNumber(noteNumber);
                this->p_04->setNoteNumber(noteNumber);
            }
            
            Index getIsMuted()  {
                return this->isMuted;
            }
            
            void setIsMuted(Index v)  {
                this->isMuted = v;
            }
            
            void onSampleRateChanged(double ) {}
            
            void extractState(PatcherStateInterface& ) {}
            
            void applyState() {
            
                this->p_01->setEngineAndPatcher(this->getEngine(), this);
                this->p_01->initialize();
                this->p_01->setParameterOffset(this->getParameterOffset(this->p_01));
            
                this->p_02->setEngineAndPatcher(this->getEngine(), this);
                this->p_02->initialize();
                this->p_02->setParameterOffset(this->getParameterOffset(this->p_02));
            
                this->p_03->setEngineAndPatcher(this->getEngine(), this);
                this->p_03->initialize();
                this->p_03->setParameterOffset(this->getParameterOffset(this->p_03));
            
                this->p_04->setEngineAndPatcher(this->getEngine(), this);
                this->p_04->initialize();
                this->p_04->setParameterOffset(this->getParameterOffset(this->p_04));
            }
            
            ParameterIndex getParameterOffset(BaseInterface& subpatcher) const {
                if (addressOf(subpatcher) == addressOf(this->p_01))
                    return 5;
            
                if (addressOf(subpatcher) == addressOf(this->p_02))
                    return 5 + this->p_01->getNumParameters();
            
                if (addressOf(subpatcher) == addressOf(this->p_03))
                    return 5 + this->p_01->getNumParameters() + this->p_02->getNumParameters();
            
                if (addressOf(subpatcher) == addressOf(this->p_04))
                    return 5 + this->p_01->getNumParameters() + this->p_02->getNumParameters() + this->p_03->getNumParameters();
            
                return 0;
            }
            
            void setParameterOffset(ParameterIndex offset) {
                this->parameterOffset = offset;
            }
            
            void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}
            
            void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}
            
            void processOutletEvent(
                EngineLink* sender,
                OutletIndex index,
                ParameterValue value,
                MillisecondTime time
            ) {
                this->updateTime(time, (ENGINE*)nullptr);
                this->processOutletAtCurrentTime(sender, index, value);
            }
            
            void sendOutlet(OutletIndex index, ParameterValue value) {
                this->getEngine()->sendOutlet(this, index, value);
            }
            
            void startup() {
                this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
                this->p_01->startup();
                this->p_02->startup();
                this->p_03->startup();
                this->p_04->startup();
            
                {
                    this->scheduleParamInit(0, 0);
                }
            
                {
                    this->scheduleParamInit(1, 0);
                }
            
                {
                    this->scheduleParamInit(2, 0);
                }
            
                {
                    this->scheduleParamInit(3, 0);
                }
            
                {
                    this->scheduleParamInit(4, 0);
                }
            }
            
            void fillDataRef(DataRefIndex , DataRef& ) {}
            
            void allocateDataRefs() {
                this->p_01->allocateDataRefs();
                this->p_02->allocateDataRefs();
                this->p_03->allocateDataRefs();
                this->p_04->allocateDataRefs();
                this->cycle_tilde_03_buffer->requestSize(16384, 1);
                this->cycle_tilde_03_buffer->setSampleRate(this->sr);
                this->cycle_tilde_03_buffer = this->cycle_tilde_03_buffer->allocateIfNeeded();
            }
            
            number param_08_value_constrain(number v) const {
                v = (v > 4 ? 4 : (v < 0 ? 0 : v));
            
                {
                    number oneStep = (number)4 / (number)4;
                    number oneStepInv = (oneStep != 0 ? (number)1 / oneStep : 0);
                    number numberOfSteps = rnbo_fround((v - 0) * oneStepInv * 1 / (number)1) * 1;
                    v = numberOfSteps * oneStep + 0;
                }
            
                return v;
            }
            
            void selector_03_onoff_set(number v) {
                this->selector_03_onoff = v;
            }
            
            void expr_08_out1_set(number v) {
                this->expr_08_out1 = v;
                this->selector_03_onoff_set(this->expr_08_out1);
            }
            
            void expr_08_in1_set(number in1) {
                this->expr_08_in1 = in1;
                this->expr_08_out1_set(this->expr_08_in1 + this->expr_08_in2);//#map:osc.analog[1]/+_obj-4:1
            }
            
            number param_09_value_constrain(number v) const {
                v = (v > 1 ? 1 : (v < 0 ? 0 : v));
            
                {
                    number oneStep = (number)1 / (number)1;
                    number oneStepInv = (oneStep != 0 ? (number)1 / oneStep : 0);
                    number numberOfSteps = rnbo_fround((v - 0) * oneStepInv * 1 / (number)1) * 1;
                    v = numberOfSteps * oneStep + 0;
                }
            
                return v;
            }
            
            void ip_05_value_set(number v) {
                this->ip_05_value = v;
                this->ip_05_fillSigBuf();
                this->ip_05_lastValue = v;
            }
            
            number param_10_value_constrain(number v) const {
                v = (v > 100 ? 100 : (v < 0 ? 0 : v));
                return v;
            }
            
            void ip_06_value_set(number v) {
                this->ip_06_value = v;
                this->ip_06_fillSigBuf();
                this->ip_06_lastValue = v;
            }
            
            number param_11_value_constrain(number v) const {
                v = (v > 100 ? 100 : (v < 0 ? 0 : v));
                return v;
            }
            
            void ip_07_value_set(number v) {
                this->ip_07_value = v;
                this->ip_07_fillSigBuf();
                this->ip_07_lastValue = v;
            }
            
            number param_12_value_constrain(number v) const {
                v = (v > 1 ? 1 : (v < 0 ? 0 : v));
                return v;
            }
            
            void p_04_in2_number_set(number v) {
                this->p_04->updateTime(this->_currentTime, (ENGINE*)nullptr);
                this->p_04->eventinlet_01_out1_number_set(v);
            }
            
            void ctlin_08_outchannel_set(number ) {}
            
            void ctlin_08_outcontroller_set(number ) {}
            
            void fromnormalized_08_output_set(number v) {
                this->param_08_value_set(v);
            }
            
            void fromnormalized_08_input_set(number v) {
                this->fromnormalized_08_output_set(this->fromnormalized(0, v));
            }
            
            void expr_09_out1_set(number v) {
                this->expr_09_out1 = v;
                this->fromnormalized_08_input_set(this->expr_09_out1);
            }
            
            void expr_09_in1_set(number in1) {
                this->expr_09_in1 = in1;
                this->expr_09_out1_set(this->expr_09_in1 * this->expr_09_in2);//#map:expr_09:1
            }
            
            void ctlin_08_value_set(number v) {
                this->expr_09_in1_set(v);
            }
            
            void ctlin_08_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_08_channel || this->ctlin_08_channel == -1) && (data[1] == this->ctlin_08_controller || this->ctlin_08_controller == -1)) {
                    this->ctlin_08_outchannel_set(channel);
                    this->ctlin_08_outcontroller_set(data[1]);
                    this->ctlin_08_value_set(data[2]);
                    this->ctlin_08_status = 0;
                }
            }
            
            void ctlin_09_outchannel_set(number ) {}
            
            void ctlin_09_outcontroller_set(number ) {}
            
            void fromnormalized_09_output_set(number v) {
                this->param_09_value_set(v);
            }
            
            void fromnormalized_09_input_set(number v) {
                this->fromnormalized_09_output_set(this->fromnormalized(1, v));
            }
            
            void expr_10_out1_set(number v) {
                this->expr_10_out1 = v;
                this->fromnormalized_09_input_set(this->expr_10_out1);
            }
            
            void expr_10_in1_set(number in1) {
                this->expr_10_in1 = in1;
                this->expr_10_out1_set(this->expr_10_in1 * this->expr_10_in2);//#map:expr_10:1
            }
            
            void ctlin_09_value_set(number v) {
                this->expr_10_in1_set(v);
            }
            
            void ctlin_09_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_09_channel || this->ctlin_09_channel == -1) && (data[1] == this->ctlin_09_controller || this->ctlin_09_controller == -1)) {
                    this->ctlin_09_outchannel_set(channel);
                    this->ctlin_09_outcontroller_set(data[1]);
                    this->ctlin_09_value_set(data[2]);
                    this->ctlin_09_status = 0;
                }
            }
            
            void ctlin_10_outchannel_set(number ) {}
            
            void ctlin_10_outcontroller_set(number ) {}
            
            void fromnormalized_10_output_set(number v) {
                this->param_10_value_set(v);
            }
            
            void fromnormalized_10_input_set(number v) {
                this->fromnormalized_10_output_set(this->fromnormalized(2, v));
            }
            
            void expr_11_out1_set(number v) {
                this->expr_11_out1 = v;
                this->fromnormalized_10_input_set(this->expr_11_out1);
            }
            
            void expr_11_in1_set(number in1) {
                this->expr_11_in1 = in1;
                this->expr_11_out1_set(this->expr_11_in1 * this->expr_11_in2);//#map:expr_11:1
            }
            
            void ctlin_10_value_set(number v) {
                this->expr_11_in1_set(v);
            }
            
            void ctlin_10_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_10_channel || this->ctlin_10_channel == -1) && (data[1] == this->ctlin_10_controller || this->ctlin_10_controller == -1)) {
                    this->ctlin_10_outchannel_set(channel);
                    this->ctlin_10_outcontroller_set(data[1]);
                    this->ctlin_10_value_set(data[2]);
                    this->ctlin_10_status = 0;
                }
            }
            
            void ctlin_11_outchannel_set(number ) {}
            
            void ctlin_11_outcontroller_set(number ) {}
            
            void fromnormalized_11_output_set(number v) {
                this->param_11_value_set(v);
            }
            
            void fromnormalized_11_input_set(number v) {
                this->fromnormalized_11_output_set(this->fromnormalized(3, v));
            }
            
            void expr_12_out1_set(number v) {
                this->expr_12_out1 = v;
                this->fromnormalized_11_input_set(this->expr_12_out1);
            }
            
            void expr_12_in1_set(number in1) {
                this->expr_12_in1 = in1;
                this->expr_12_out1_set(this->expr_12_in1 * this->expr_12_in2);//#map:expr_12:1
            }
            
            void ctlin_11_value_set(number v) {
                this->expr_12_in1_set(v);
            }
            
            void ctlin_11_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_11_channel || this->ctlin_11_channel == -1) && (data[1] == this->ctlin_11_controller || this->ctlin_11_controller == -1)) {
                    this->ctlin_11_outchannel_set(channel);
                    this->ctlin_11_outcontroller_set(data[1]);
                    this->ctlin_11_value_set(data[2]);
                    this->ctlin_11_status = 0;
                }
            }
            
            void ctlin_12_outchannel_set(number ) {}
            
            void ctlin_12_outcontroller_set(number ) {}
            
            void fromnormalized_12_output_set(number v) {
                this->param_12_value_set(v);
            }
            
            void fromnormalized_12_input_set(number v) {
                this->fromnormalized_12_output_set(this->fromnormalized(4, v));
            }
            
            void expr_13_out1_set(number v) {
                this->expr_13_out1 = v;
                this->fromnormalized_12_input_set(this->expr_13_out1);
            }
            
            void expr_13_in1_set(number in1) {
                this->expr_13_in1 = in1;
                this->expr_13_out1_set(this->expr_13_in1 * this->expr_13_in2);//#map:expr_13:1
            }
            
            void ctlin_12_value_set(number v) {
                this->expr_13_in1_set(v);
            }
            
            void ctlin_12_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
                RNBO_UNUSED(length);
                RNBO_UNUSED(port);
            
                if (status == 0xB0 && (channel == this->ctlin_12_channel || this->ctlin_12_channel == -1) && (data[1] == this->ctlin_12_controller || this->ctlin_12_controller == -1)) {
                    this->ctlin_12_outchannel_set(channel);
                    this->ctlin_12_outcontroller_set(data[1]);
                    this->ctlin_12_value_set(data[2]);
                    this->ctlin_12_status = 0;
                }
            }
            
            void ip_05_perform(SampleValue * out, Index n) {
                auto __ip_05_lastValue = this->ip_05_lastValue;
                auto __ip_05_lastIndex = this->ip_05_lastIndex;
            
                for (Index i = 0; i < n; i++) {
                    out[(Index)i] = ((SampleIndex)(i) >= __ip_05_lastIndex ? __ip_05_lastValue : this->ip_05_sigbuf[(Index)i]);
                }
            
                __ip_05_lastIndex = 0;
                this->ip_05_lastIndex = __ip_05_lastIndex;
            }
            
            void ip_06_perform(SampleValue * out, Index n) {
                auto __ip_06_lastValue = this->ip_06_lastValue;
                auto __ip_06_lastIndex = this->ip_06_lastIndex;
            
                for (Index i = 0; i < n; i++) {
                    out[(Index)i] = ((SampleIndex)(i) >= __ip_06_lastIndex ? __ip_06_lastValue : this->ip_06_sigbuf[(Index)i]);
                }
            
                __ip_06_lastIndex = 0;
                this->ip_06_lastIndex = __ip_06_lastIndex;
            }
            
            void ip_07_perform(SampleValue * out, Index n) {
                auto __ip_07_lastValue = this->ip_07_lastValue;
                auto __ip_07_lastIndex = this->ip_07_lastIndex;
            
                for (Index i = 0; i < n; i++) {
                    out[(Index)i] = ((SampleIndex)(i) >= __ip_07_lastIndex ? __ip_07_lastValue : this->ip_07_sigbuf[(Index)i]);
                }
            
                __ip_07_lastIndex = 0;
                this->ip_07_lastIndex = __ip_07_lastIndex;
            }
            
            void phasor_03_perform(const Sample * freq, SampleValue * out, Index n) {
                for (Index i = 0; i < n; i++) {
                    out[(Index)i] = this->phasor_03_ph_next(freq[(Index)i], -1);
                    this->phasor_03_sigbuf[(Index)i] = -1;
                }
            }
            
            void p_03_perform(const SampleValue * in1, SampleValue * out1, Index n) {
                // subpatcher: square
                ConstSampleArray<1> ins = {in1};
            
                SampleArray<1> outs = {out1};
                this->p_03->process(ins, 1, outs, 1, n);
            }
            
            void p_04_perform(const SampleValue * in1, SampleValue * out1, Index n) {
                // subpatcher: rect
                ConstSampleArray<1> ins = {in1};
            
                SampleArray<1> outs = {out1};
                this->p_04->process(ins, 1, outs, 1, n);
            }
            
            void phasor_04_perform(const Sample * freq, SampleValue * out, Index n) {
                for (Index i = 0; i < n; i++) {
                    out[(Index)i] = this->phasor_04_ph_next(freq[(Index)i], -1);
                    this->phasor_04_sigbuf[(Index)i] = -1;
                }
            }
            
            void triangle_tilde_01_perform(const Sample * phase, number duty, SampleValue * out1, Index n) {
                RNBO_UNUSED(duty);
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    number p1 = 0.5;
                    auto wrappedPhase = this->wrap(phase[(Index)i], 0., 1.);
                    p1 = (p1 > 1. ? 1. : (p1 < 0. ? 0. : p1));
            
                    if (wrappedPhase < p1) {
                        out1[(Index)i] = wrappedPhase / p1;
                        continue;
                    } else {
                        out1[(Index)i] = (p1 == 1. ? wrappedPhase : 1. - (wrappedPhase - p1) / (1. - p1));
                        continue;
                    }
                }
            }
            
            void cycle_tilde_03_perform(
                const Sample * frequency,
                number phase_offset,
                SampleValue * out1,
                SampleValue * out2,
                Index n
            ) {
                RNBO_UNUSED(phase_offset);
                auto __cycle_tilde_03_f2i = this->cycle_tilde_03_f2i;
                auto __cycle_tilde_03_buffer = this->cycle_tilde_03_buffer;
                auto __cycle_tilde_03_phasei = this->cycle_tilde_03_phasei;
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    {
                        UInt32 uint_phase;
            
                        {
                            {
                                uint_phase = __cycle_tilde_03_phasei;
                            }
                        }
            
                        UInt32 idx = (UInt32)(uint32_rshift(uint_phase, 18));
                        number frac = ((BinOpInt)((BinOpInt)uint_phase & (BinOpInt)262143)) * 3.81471181759574e-6;
                        number y0 = __cycle_tilde_03_buffer[(Index)idx];
                        number y1 = __cycle_tilde_03_buffer[(Index)((BinOpInt)(idx + 1) & (BinOpInt)16383)];
                        number y = y0 + frac * (y1 - y0);
            
                        {
                            UInt32 pincr = (UInt32)(uint32_trunc(frequency[(Index)i] * __cycle_tilde_03_f2i));
                            __cycle_tilde_03_phasei = uint32_add(__cycle_tilde_03_phasei, pincr);
                        }
            
                        out1[(Index)i] = y;
                        out2[(Index)i] = uint_phase * 0.232830643653869629e-9;
                        continue;
                    }
                }
            
                this->cycle_tilde_03_phasei = __cycle_tilde_03_phasei;
            }
            
            void scale_tilde_01_perform(
                const Sample * x,
                number lowin,
                number hiin,
                number lowout,
                number highout,
                number pow,
                SampleValue * out1,
                Index n
            ) {
                RNBO_UNUSED(pow);
                RNBO_UNUSED(highout);
                RNBO_UNUSED(lowout);
                RNBO_UNUSED(hiin);
                RNBO_UNUSED(lowin);
                auto inscale = this->safediv(1., 1 - -1);
                number outdiff = 1 - 0;
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    number value = (x[(Index)i] - -1) * inscale;
                    value = value * outdiff + 0;
                    out1[(Index)i] = value;
                }
            }
            
            void selector_03_perform(
                number onoff,
                const SampleValue * in1,
                const SampleValue * in2,
                const SampleValue * in3,
                const SampleValue * in4,
                const SampleValue * in5,
                SampleValue * out,
                Index n
            ) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    if (onoff >= 1 && onoff < 2)
                        out[(Index)i] = in1[(Index)i];
                    else if (onoff >= 2 && onoff < 3)
                        out[(Index)i] = in2[(Index)i];
                    else if (onoff >= 3 && onoff < 4)
                        out[(Index)i] = in3[(Index)i];
                    else if (onoff >= 4 && onoff < 5)
                        out[(Index)i] = in4[(Index)i];
                    else if (onoff >= 5 && onoff < 6)
                        out[(Index)i] = in5[(Index)i];
                    else
                        out[(Index)i] = 0;
                }
            }
            
            void p_02_perform(
                const SampleValue * in1,
                const SampleValue * in2,
                SampleValue * out1,
                Index n
            ) {
                // subpatcher: jitter
                ConstSampleArray<2> ins = {in1, in2};
            
                SampleArray<1> outs = {out1};
                this->p_02->process(ins, 2, outs, 1, n);
            }
            
            void p_01_perform(
                const SampleValue * in1,
                const SampleValue * in2,
                SampleValue * out1,
                Index n
            ) {
                // subpatcher: smooth
                ConstSampleArray<2> ins = {in1, in2};
            
                SampleArray<1> outs = {out1};
                this->p_01->process(ins, 2, outs, 1, n);
            }
            
            void dspexpr_17_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
                Index i;
            
                for (i = 0; i < (Index)n; i++) {
                    out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
                }
            }
            
            void stackprotect_perform(Index n) {
                RNBO_UNUSED(n);
                auto __stackprotect_count = this->stackprotect_count;
                __stackprotect_count = 0;
                this->stackprotect_count = __stackprotect_count;
            }
            
            void param_08_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_08_value;
            }
            
            void param_08_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_08_value_set(preset["value"]);
            }
            
            void ip_05_init() {
                this->ip_05_lastValue = this->ip_05_value;
            }
            
            void ip_05_fillSigBuf() {
                if ((bool)(this->ip_05_sigbuf)) {
                    SampleIndex k = (SampleIndex)(this->sampleOffsetIntoNextAudioBuffer);
            
                    if (k >= (SampleIndex)(this->vs))
                        k = (SampleIndex)(this->vs) - 1;
            
                    for (SampleIndex i = (SampleIndex)(this->ip_05_lastIndex); i < k; i++) {
                        if (this->ip_05_resetCount > 0) {
                            this->ip_05_sigbuf[(Index)i] = 1;
                            this->ip_05_resetCount--;
                        } else {
                            this->ip_05_sigbuf[(Index)i] = this->ip_05_lastValue;
                        }
                    }
            
                    this->ip_05_lastIndex = k;
                }
            }
            
            void ip_05_dspsetup(bool force) {
                if ((bool)(this->ip_05_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->ip_05_lastIndex = 0;
                this->ip_05_setupDone = true;
            }
            
            void param_09_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_09_value;
            }
            
            void param_09_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_09_value_set(preset["value"]);
            }
            
            number cycle_tilde_03_ph_next(number freq, number reset) {
                {
                    {
                        if (reset >= 0.)
                            this->cycle_tilde_03_ph_currentPhase = reset;
                    }
                }
            
                number pincr = freq * this->cycle_tilde_03_ph_conv;
            
                if (this->cycle_tilde_03_ph_currentPhase < 0.)
                    this->cycle_tilde_03_ph_currentPhase = 1. + this->cycle_tilde_03_ph_currentPhase;
            
                if (this->cycle_tilde_03_ph_currentPhase > 1.)
                    this->cycle_tilde_03_ph_currentPhase = this->cycle_tilde_03_ph_currentPhase - 1.;
            
                number tmp = this->cycle_tilde_03_ph_currentPhase;
                this->cycle_tilde_03_ph_currentPhase += pincr;
                return tmp;
            }
            
            void cycle_tilde_03_ph_reset() {
                this->cycle_tilde_03_ph_currentPhase = 0;
            }
            
            void cycle_tilde_03_ph_dspsetup() {
                this->cycle_tilde_03_ph_conv = (number)1 / this->sr;
            }
            
            void cycle_tilde_03_dspsetup(bool force) {
                if ((bool)(this->cycle_tilde_03_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->cycle_tilde_03_phasei = 0;
                this->cycle_tilde_03_f2i = (number)4294967296 / this->sr;
                this->cycle_tilde_03_wrap = (Int)(this->cycle_tilde_03_buffer->getSize()) - 1;
                this->cycle_tilde_03_setupDone = true;
                this->cycle_tilde_03_ph_dspsetup();
            }
            
            void cycle_tilde_03_bufferUpdated() {
                this->cycle_tilde_03_wrap = (Int)(this->cycle_tilde_03_buffer->getSize()) - 1;
            }
            
            void ip_06_init() {
                this->ip_06_lastValue = this->ip_06_value;
            }
            
            void ip_06_fillSigBuf() {
                if ((bool)(this->ip_06_sigbuf)) {
                    SampleIndex k = (SampleIndex)(this->sampleOffsetIntoNextAudioBuffer);
            
                    if (k >= (SampleIndex)(this->vs))
                        k = (SampleIndex)(this->vs) - 1;
            
                    for (SampleIndex i = (SampleIndex)(this->ip_06_lastIndex); i < k; i++) {
                        if (this->ip_06_resetCount > 0) {
                            this->ip_06_sigbuf[(Index)i] = 1;
                            this->ip_06_resetCount--;
                        } else {
                            this->ip_06_sigbuf[(Index)i] = this->ip_06_lastValue;
                        }
                    }
            
                    this->ip_06_lastIndex = k;
                }
            }
            
            void ip_06_dspsetup(bool force) {
                if ((bool)(this->ip_06_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->ip_06_lastIndex = 0;
                this->ip_06_setupDone = true;
            }
            
            void param_10_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_10_value;
            }
            
            void param_10_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_10_value_set(preset["value"]);
            }
            
            void ip_07_init() {
                this->ip_07_lastValue = this->ip_07_value;
            }
            
            void ip_07_fillSigBuf() {
                if ((bool)(this->ip_07_sigbuf)) {
                    SampleIndex k = (SampleIndex)(this->sampleOffsetIntoNextAudioBuffer);
            
                    if (k >= (SampleIndex)(this->vs))
                        k = (SampleIndex)(this->vs) - 1;
            
                    for (SampleIndex i = (SampleIndex)(this->ip_07_lastIndex); i < k; i++) {
                        if (this->ip_07_resetCount > 0) {
                            this->ip_07_sigbuf[(Index)i] = 1;
                            this->ip_07_resetCount--;
                        } else {
                            this->ip_07_sigbuf[(Index)i] = this->ip_07_lastValue;
                        }
                    }
            
                    this->ip_07_lastIndex = k;
                }
            }
            
            void ip_07_dspsetup(bool force) {
                if ((bool)(this->ip_07_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->ip_07_lastIndex = 0;
                this->ip_07_setupDone = true;
            }
            
            void param_11_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_11_value;
            }
            
            void param_11_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_11_value_set(preset["value"]);
            }
            
            void phasor_03_onBeatTimeChanged(number beattime) {
                RNBO_UNUSED(beattime);
                this->phasor_03_recalcInc = true;
                this->phasor_03_recalcPhase = true;
            }
            
            number phasor_03_ph_next(number freq, number reset) {
                RNBO_UNUSED(reset);
                number pincr = freq * this->phasor_03_ph_conv;
            
                if (this->phasor_03_ph_currentPhase < 0.)
                    this->phasor_03_ph_currentPhase = 1. + this->phasor_03_ph_currentPhase;
            
                if (this->phasor_03_ph_currentPhase > 1.)
                    this->phasor_03_ph_currentPhase = this->phasor_03_ph_currentPhase - 1.;
            
                number tmp = this->phasor_03_ph_currentPhase;
                this->phasor_03_ph_currentPhase += pincr;
                return tmp;
            }
            
            void phasor_03_ph_reset() {
                this->phasor_03_ph_currentPhase = 0;
            }
            
            void phasor_03_ph_dspsetup() {
                this->phasor_03_ph_conv = (number)1 / this->sr;
            }
            
            void phasor_03_dspsetup(bool force) {
                if ((bool)(this->phasor_03_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->phasor_03_setupDone = true;
                this->phasor_03_ph_dspsetup();
            }
            
            void phasor_04_onBeatTimeChanged(number beattime) {
                RNBO_UNUSED(beattime);
                this->phasor_04_recalcInc = true;
                this->phasor_04_recalcPhase = true;
            }
            
            number phasor_04_ph_next(number freq, number reset) {
                RNBO_UNUSED(reset);
                number pincr = freq * this->phasor_04_ph_conv;
            
                if (this->phasor_04_ph_currentPhase < 0.)
                    this->phasor_04_ph_currentPhase = 1. + this->phasor_04_ph_currentPhase;
            
                if (this->phasor_04_ph_currentPhase > 1.)
                    this->phasor_04_ph_currentPhase = this->phasor_04_ph_currentPhase - 1.;
            
                number tmp = this->phasor_04_ph_currentPhase;
                this->phasor_04_ph_currentPhase += pincr;
                return tmp;
            }
            
            void phasor_04_ph_reset() {
                this->phasor_04_ph_currentPhase = 0;
            }
            
            void phasor_04_ph_dspsetup() {
                this->phasor_04_ph_conv = (number)1 / this->sr;
            }
            
            void phasor_04_dspsetup(bool force) {
                if ((bool)(this->phasor_04_setupDone) && (bool)(!(bool)(force)))
                    return;
            
                this->phasor_04_setupDone = true;
                this->phasor_04_ph_dspsetup();
            }
            
            void param_12_getPresetValue(PatcherStateInterface& preset) {
                preset["value"] = this->param_12_value;
            }
            
            void param_12_setPresetValue(PatcherStateInterface& preset) {
                if ((bool)(stateIsEmpty(preset)))
                    return;
            
                this->param_12_value_set(preset["value"]);
            }
            
            bool stackprotect_check() {
                this->stackprotect_count++;
            
                if (this->stackprotect_count > 128) {
                    console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
                    return true;
                }
            
                return false;
            }
            
            Index getPatcherSerial() const {
                return 0;
            }
            
            void sendParameter(ParameterIndex index, bool ignoreValue) {
                this->getPatcher()->sendParameter(index + this->parameterOffset, ignoreValue);
            }
            
            void scheduleParamInit(ParameterIndex index, Index order) {
                this->getPatcher()->scheduleParamInit(index + this->parameterOffset, order);
            }
            
            void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
                RNBO_UNUSED(inProcess);
                RNBO_UNUSED(engine);
                this->_currentTime = time;
                auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));
            
                if (offset >= (SampleIndex)(this->vs))
                    offset = (SampleIndex)(this->vs) - 1;
            
                if (offset < 0)
                    offset = 0;
            
                this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
            }
            
            void assign_defaults()
            {
                dspexpr_17_in1 = 0;
                dspexpr_17_in2 = 0;
                expr_08_in1 = 0;
                expr_08_in2 = 1;
                expr_08_out1 = 0;
                param_08_value = 0;
                selector_03_onoff = 1;
                p_01_target = 0;
                p_02_target = 0;
                ip_05_value = 0;
                ip_05_impulse = 0;
                param_09_value = 1;
                scale_tilde_01_x = 0;
                scale_tilde_01_lowin = -1;
                scale_tilde_01_hiin = 1;
                scale_tilde_01_lowout = 0;
                scale_tilde_01_highout = 1;
                scale_tilde_01_pow = 1;
                cycle_tilde_03_frequency = 0;
                cycle_tilde_03_phase_offset = 0;
                ip_06_value = 0;
                ip_06_impulse = 0;
                param_10_value = 0;
                ip_07_value = 0;
                ip_07_impulse = 0;
                param_11_value = 0;
                phasor_03_freq = 0;
                p_03_target = 0;
                p_04_target = 0;
                phasor_04_freq = 0;
                triangle_tilde_01_phase = 0;
                triangle_tilde_01_duty = 0.5;
                param_12_value = 0.8;
                ctlin_08_input = 0;
                ctlin_08_controller = 0;
                ctlin_08_channel = -1;
                expr_09_in1 = 0;
                expr_09_in2 = 0.007874015748;
                expr_09_out1 = 0;
                ctlin_09_input = 0;
                ctlin_09_controller = 0;
                ctlin_09_channel = -1;
                expr_10_in1 = 0;
                expr_10_in2 = 0.007874015748;
                expr_10_out1 = 0;
                ctlin_10_input = 0;
                ctlin_10_controller = 0;
                ctlin_10_channel = -1;
                expr_11_in1 = 0;
                expr_11_in2 = 0.007874015748;
                expr_11_out1 = 0;
                ctlin_11_input = 0;
                ctlin_11_controller = 0;
                ctlin_11_channel = -1;
                expr_12_in1 = 0;
                expr_12_in2 = 0.007874015748;
                expr_12_out1 = 0;
                ctlin_12_input = 0;
                ctlin_12_controller = 0;
                ctlin_12_channel = -1;
                expr_13_in1 = 0;
                expr_13_in2 = 0.007874015748;
                expr_13_out1 = 0;
                _currentTime = 0;
                audioProcessSampleCount = 0;
                sampleOffsetIntoNextAudioBuffer = 0;
                zeroBuffer = nullptr;
                dummyBuffer = nullptr;
                signals[0] = nullptr;
                signals[1] = nullptr;
                signals[2] = nullptr;
                signals[3] = nullptr;
                signals[4] = nullptr;
                signals[5] = nullptr;
                signals[6] = nullptr;
                signals[7] = nullptr;
                signals[8] = nullptr;
                didAllocateSignals = 0;
                vs = 0;
                maxvs = 0;
                sr = 44100;
                invsr = 0.000022675736961451248;
                param_08_lastValue = 0;
                ip_05_lastIndex = 0;
                ip_05_lastValue = 0;
                ip_05_resetCount = 0;
                ip_05_sigbuf = nullptr;
                ip_05_setupDone = false;
                param_09_lastValue = 0;
                cycle_tilde_03_wrap = 0;
                cycle_tilde_03_ph_currentPhase = 0;
                cycle_tilde_03_ph_conv = 0;
                cycle_tilde_03_setupDone = false;
                ip_06_lastIndex = 0;
                ip_06_lastValue = 0;
                ip_06_resetCount = 0;
                ip_06_sigbuf = nullptr;
                ip_06_setupDone = false;
                param_10_lastValue = 0;
                ip_07_lastIndex = 0;
                ip_07_lastValue = 0;
                ip_07_resetCount = 0;
                ip_07_sigbuf = nullptr;
                ip_07_setupDone = false;
                param_11_lastValue = 0;
                phasor_03_sigbuf = nullptr;
                phasor_03_lastLockedPhase = 0;
                phasor_03_lastQuantum = 0;
                phasor_03_lastTempo = 0;
                phasor_03_nextJumpInSamples = 0;
                phasor_03_inc = 0;
                phasor_03_recalcInc = true;
                phasor_03_recalcPhase = true;
                phasor_03_ph_currentPhase = 0;
                phasor_03_ph_conv = 0;
                phasor_03_setupDone = false;
                phasor_04_sigbuf = nullptr;
                phasor_04_lastLockedPhase = 0;
                phasor_04_lastQuantum = 0;
                phasor_04_lastTempo = 0;
                phasor_04_nextJumpInSamples = 0;
                phasor_04_inc = 0;
                phasor_04_recalcInc = true;
                phasor_04_recalcPhase = true;
                phasor_04_ph_currentPhase = 0;
                phasor_04_ph_conv = 0;
                phasor_04_setupDone = false;
                param_12_lastValue = 0;
                ctlin_08_status = 0;
                ctlin_08_byte1 = -1;
                ctlin_08_inchan = 0;
                ctlin_09_status = 0;
                ctlin_09_byte1 = -1;
                ctlin_09_inchan = 0;
                ctlin_10_status = 0;
                ctlin_10_byte1 = -1;
                ctlin_10_inchan = 0;
                ctlin_11_status = 0;
                ctlin_11_byte1 = -1;
                ctlin_11_inchan = 0;
                ctlin_12_status = 0;
                ctlin_12_byte1 = -1;
                ctlin_12_inchan = 0;
                stackprotect_count = 0;
                _voiceIndex = 0;
                _noteNumber = 0;
                isMuted = 1;
                parameterOffset = 0;
            }
            
            // member variables
            
                number dspexpr_17_in1;
                number dspexpr_17_in2;
                number expr_08_in1;
                number expr_08_in2;
                number expr_08_out1;
                number param_08_value;
                number selector_03_onoff;
                number p_01_target;
                number p_02_target;
                number ip_05_value;
                number ip_05_impulse;
                number param_09_value;
                number scale_tilde_01_x;
                number scale_tilde_01_lowin;
                number scale_tilde_01_hiin;
                number scale_tilde_01_lowout;
                number scale_tilde_01_highout;
                number scale_tilde_01_pow;
                number cycle_tilde_03_frequency;
                number cycle_tilde_03_phase_offset;
                number ip_06_value;
                number ip_06_impulse;
                number param_10_value;
                number ip_07_value;
                number ip_07_impulse;
                number param_11_value;
                number phasor_03_freq;
                number p_03_target;
                number p_04_target;
                number phasor_04_freq;
                number triangle_tilde_01_phase;
                number triangle_tilde_01_duty;
                number param_12_value;
                number ctlin_08_input;
                number ctlin_08_controller;
                number ctlin_08_channel;
                number expr_09_in1;
                number expr_09_in2;
                number expr_09_out1;
                number ctlin_09_input;
                number ctlin_09_controller;
                number ctlin_09_channel;
                number expr_10_in1;
                number expr_10_in2;
                number expr_10_out1;
                number ctlin_10_input;
                number ctlin_10_controller;
                number ctlin_10_channel;
                number expr_11_in1;
                number expr_11_in2;
                number expr_11_out1;
                number ctlin_11_input;
                number ctlin_11_controller;
                number ctlin_11_channel;
                number expr_12_in1;
                number expr_12_in2;
                number expr_12_out1;
                number ctlin_12_input;
                number ctlin_12_controller;
                number ctlin_12_channel;
                number expr_13_in1;
                number expr_13_in2;
                number expr_13_out1;
                MillisecondTime _currentTime;
                UInt64 audioProcessSampleCount;
                Index sampleOffsetIntoNextAudioBuffer;
                signal zeroBuffer;
                signal dummyBuffer;
                SampleValue * signals[9];
                bool didAllocateSignals;
                Index vs;
                Index maxvs;
                number sr;
                number invsr;
                number param_08_lastValue;
                SampleIndex ip_05_lastIndex;
                number ip_05_lastValue;
                SampleIndex ip_05_resetCount;
                signal ip_05_sigbuf;
                bool ip_05_setupDone;
                number param_09_lastValue;
                SampleBufferRef cycle_tilde_03_buffer;
                Int cycle_tilde_03_wrap;
                UInt32 cycle_tilde_03_phasei;
                SampleValue cycle_tilde_03_f2i;
                number cycle_tilde_03_ph_currentPhase;
                number cycle_tilde_03_ph_conv;
                bool cycle_tilde_03_setupDone;
                SampleIndex ip_06_lastIndex;
                number ip_06_lastValue;
                SampleIndex ip_06_resetCount;
                signal ip_06_sigbuf;
                bool ip_06_setupDone;
                number param_10_lastValue;
                SampleIndex ip_07_lastIndex;
                number ip_07_lastValue;
                SampleIndex ip_07_resetCount;
                signal ip_07_sigbuf;
                bool ip_07_setupDone;
                number param_11_lastValue;
                signal phasor_03_sigbuf;
                number phasor_03_lastLockedPhase;
                number phasor_03_lastQuantum;
                number phasor_03_lastTempo;
                number phasor_03_nextJumpInSamples;
                number phasor_03_inc;
                bool phasor_03_recalcInc;
                bool phasor_03_recalcPhase;
                number phasor_03_ph_currentPhase;
                number phasor_03_ph_conv;
                bool phasor_03_setupDone;
                signal phasor_04_sigbuf;
                number phasor_04_lastLockedPhase;
                number phasor_04_lastQuantum;
                number phasor_04_lastTempo;
                number phasor_04_nextJumpInSamples;
                number phasor_04_inc;
                bool phasor_04_recalcInc;
                bool phasor_04_recalcPhase;
                number phasor_04_ph_currentPhase;
                number phasor_04_ph_conv;
                bool phasor_04_setupDone;
                number param_12_lastValue;
                Int ctlin_08_status;
                Int ctlin_08_byte1;
                Int ctlin_08_inchan;
                Int ctlin_09_status;
                Int ctlin_09_byte1;
                Int ctlin_09_inchan;
                Int ctlin_10_status;
                Int ctlin_10_byte1;
                Int ctlin_10_inchan;
                Int ctlin_11_status;
                Int ctlin_11_byte1;
                Int ctlin_11_inchan;
                Int ctlin_12_status;
                Int ctlin_12_byte1;
                Int ctlin_12_inchan;
                number stackprotect_count;
                Index _voiceIndex;
                Int _noteNumber;
                Index isMuted;
                ParameterIndex parameterOffset;
                RNBOSubpatcher_945 p_01;
                RNBOSubpatcher_946 p_02;
                RNBOSubpatcher_947 p_03;
                RNBOSubpatcher_948 p_04;
                bool _isInitialized = false;
    };
    
    void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
    	if (time == TimeNow) time = getTopLevelPatcher()->getPatcherTime();
    	getTopLevelPatcher()->processInternalEvents(time);
    	updateTime(time, (EXTERNALENGINE*)nullptr);
    }
    
    RNBOSubpatcher_953* operator->() {
        return this;
    }
    const RNBOSubpatcher_953* operator->() const {
        return this;
    }
    virtual rnbomatic* getPatcher() const {
        return static_cast<rnbomatic *>(_parentPatcher);
    }
    
    rnbomatic* getTopLevelPatcher() {
        return this->getPatcher()->getTopLevelPatcher();
    }
    
    void cancelClockEvents()
    {
    }
    
    inline number linearinterp(number frac, number x, number y) {
        return x + (y - x) * frac;
    }
    
    inline number safediv(number num, number denom) {
        return (denom == 0.0 ? 0.0 : num / denom);
    }
    
    number maximum(number x, number y) {
        return (x < y ? y : x);
    }
    
    inline number safesqrt(number num) {
        return (num > 0.0 ? rnbo_sqrt(num) : 0.0);
    }
    
    number fromnormalized(Index index, number normalizedValue) {
        return this->convertFromNormalizedParameterValue(index, normalizedValue);
    }
    
    MillisecondTime getPatcherTime() const {
        return this->_currentTime;
    }
    
    void param_13_value_set(number v) {
        v = this->param_13_value_constrain(v);
        this->param_13_value = v;
        this->sendParameter(0, false);
    
        if (this->param_13_value != this->param_13_lastValue) {
            this->getEngine()->presetTouched();
            this->param_13_lastValue = this->param_13_value;
        }
    
        this->slide_tilde_03_x_set(v);
    }
    
    void param_14_value_set(number v) {
        v = this->param_14_value_constrain(v);
        this->param_14_value = v;
        this->sendParameter(1, false);
    
        if (this->param_14_value != this->param_14_lastValue) {
            this->getEngine()->presetTouched();
            this->param_14_lastValue = this->param_14_value;
        }
    
        this->slide_tilde_04_x_set(v);
    }
    
    void param_15_value_set(number v) {
        v = this->param_15_value_constrain(v);
        this->param_15_value = v;
        this->sendParameter(2, false);
    
        if (this->param_15_value != this->param_15_lastValue) {
            this->getEngine()->presetTouched();
            this->param_15_lastValue = this->param_15_value;
        }
    
        this->slide_tilde_07_x_set(v);
    }
    
    void param_16_value_set(number v) {
        v = this->param_16_value_constrain(v);
        this->param_16_value = v;
        this->sendParameter(3, false);
    
        if (this->param_16_value != this->param_16_lastValue) {
            this->getEngine()->presetTouched();
            this->param_16_lastValue = this->param_16_value;
        }
    
        this->slide_tilde_10_x_set(v);
    }
    
    void param_17_value_set(number v) {
        v = this->param_17_value_constrain(v);
        this->param_17_value = v;
        this->sendParameter(4, false);
    
        if (this->param_17_value != this->param_17_lastValue) {
            this->getEngine()->presetTouched();
            this->param_17_lastValue = this->param_17_value;
        }
    
        this->slide_tilde_13_x_set(v);
    }
    
    void param_18_value_set(number v) {
        v = this->param_18_value_constrain(v);
        this->param_18_value = v;
        this->sendParameter(5, false);
    
        if (this->param_18_value != this->param_18_lastValue) {
            this->getEngine()->presetTouched();
            this->param_18_lastValue = this->param_18_value;
        }
    
        this->slide_tilde_12_x_set(v);
    }
    
    void numberobj_01_valin_set(number v) {
        this->numberobj_01_value_set(v);
    }
    
    void numberobj_01_format_set(number v) {
        this->numberobj_01_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
    }
    
    void numberobj_02_valin_set(number v) {
        this->numberobj_02_value_set(v);
    }
    
    void numberobj_02_format_set(number v) {
        this->numberobj_02_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
    }
    
    void numberobj_03_valin_set(number v) {
        this->numberobj_03_value_set(v);
    }
    
    void numberobj_03_format_set(number v) {
        this->numberobj_03_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
    }
    
    void numberobj_04_valin_set(number v) {
        this->numberobj_04_value_set(v);
    }
    
    void numberobj_04_format_set(number v) {
        this->numberobj_04_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
    }
    
    void deallocateSignals() {
        Index i;
    
        for (i = 0; i < 12; i++) {
            this->signals[i] = freeSignal(this->signals[i]);
        }
    
        this->ip_08_sigbuf = freeSignal(this->ip_08_sigbuf);
        this->zeroBuffer = freeSignal(this->zeroBuffer);
        this->dummyBuffer = freeSignal(this->dummyBuffer);
    }
    
    Index getMaxBlockSize() const {
        return this->maxvs;
    }
    
    number getSampleRate() const {
        return this->sr;
    }
    
    bool hasFixedVectorSize() const {
        return false;
    }
    
    void setProbingTarget(MessageTag ) {}
    
    void initializeObjects() {
        this->mtof_tilde_01_innerScala_init();
        this->mtof_tilde_01_init();
        this->ip_08_init();
        this->numberobj_01_init();
        this->numberobj_02_init();
        this->numberobj_03_init();
        this->numberobj_04_init();
        this->p_05->initializeObjects();
        this->p_06->initializeObjects();
        this->p_07->initializeObjects();
        this->p_08->initializeObjects();
    }
    
    void setVoiceIndex(Index index)  {
        this->_voiceIndex = index;
        this->p_05->setVoiceIndex(index);
        this->p_06->setVoiceIndex(index);
        this->p_07->setVoiceIndex(index);
        this->p_08->setVoiceIndex(index);
    }
    
    void setNoteNumber(Int noteNumber)  {
        this->_noteNumber = noteNumber;
        this->p_05->setNoteNumber(noteNumber);
        this->p_06->setNoteNumber(noteNumber);
        this->p_07->setNoteNumber(noteNumber);
        this->p_08->setNoteNumber(noteNumber);
    }
    
    Index getIsMuted()  {
        return this->isMuted;
    }
    
    void setIsMuted(Index v)  {
        this->isMuted = v;
    }
    
    void onSampleRateChanged(double ) {}
    
    void extractState(PatcherStateInterface& ) {}
    
    void applyState() {
    
        this->p_05->setEngineAndPatcher(this->getEngine(), this);
        this->p_05->initialize();
        this->p_05->setParameterOffset(this->getParameterOffset(this->p_05));
    
        this->p_06->setEngineAndPatcher(this->getEngine(), this);
        this->p_06->initialize();
        this->p_06->setParameterOffset(this->getParameterOffset(this->p_06));
    
        this->p_07->setEngineAndPatcher(this->getEngine(), this);
        this->p_07->initialize();
        this->p_07->setParameterOffset(this->getParameterOffset(this->p_07));
    
        this->p_08->setEngineAndPatcher(this->getEngine(), this);
        this->p_08->initialize();
        this->p_08->setParameterOffset(this->getParameterOffset(this->p_08));
    }
    
    ParameterValue getPolyParameterValue(RNBOSubpatcher_953* voices, ParameterIndex index)  {
        switch (index) {
        default:
            {
            return voices[0]->getParameterValue(index);
            }
        }
    }
    
    void setPolyParameterValue(
        RNBOSubpatcher_953* voices,
        ParameterIndex index,
        ParameterValue value,
        MillisecondTime time
    ) {
        switch (index) {
        default:
            {
            for (Index i = 0; i < 4; i++)
                voices[i]->setParameterValue(index, value, time);
            }
        }
    }
    
    void sendPolyParameter(ParameterIndex index, Index voiceIndex, bool ignoreValue) {
        this->getPatcher()->sendParameter(index + this->parameterOffset + voiceIndex - 1, ignoreValue);
    }
    
    ParameterIndex getParameterOffset(BaseInterface& subpatcher) const {
        if (addressOf(subpatcher) == addressOf(this->p_05))
            return 6;
    
        if (addressOf(subpatcher) == addressOf(this->p_06))
            return 6 + this->p_05->getNumParameters();
    
        if (addressOf(subpatcher) == addressOf(this->p_07))
            return 6 + this->p_05->getNumParameters() + this->p_06->getNumParameters();
    
        if (addressOf(subpatcher) == addressOf(this->p_08))
            return 6 + this->p_05->getNumParameters() + this->p_06->getNumParameters() + this->p_07->getNumParameters();
    
        return 0;
    }
    
    void setParameterOffset(ParameterIndex offset) {
        this->parameterOffset = offset;
    }
    
    void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}
    
    void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}
    
    void processOutletEvent(
        EngineLink* sender,
        OutletIndex index,
        ParameterValue value,
        MillisecondTime time
    ) {
        this->updateTime(time, (ENGINE*)nullptr);
        this->processOutletAtCurrentTime(sender, index, value);
    }
    
    void sendOutlet(OutletIndex index, ParameterValue value) {
        this->getEngine()->sendOutlet(this, index, value);
    }
    
    void startup() {
        this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
        this->p_05->startup();
        this->p_06->startup();
        this->p_07->startup();
        this->p_08->startup();
    }
    
    void fillDataRef(DataRefIndex , DataRef& ) {}
    
    void allocateDataRefs() {
        this->p_05->allocateDataRefs();
        this->p_06->allocateDataRefs();
        this->p_07->allocateDataRefs();
        this->p_08->allocateDataRefs();
        this->mtof_tilde_01_innerMtoF_buffer->requestSize(65536, 1);
        this->mtof_tilde_01_innerMtoF_buffer->setSampleRate(this->sr);
        this->mtof_tilde_01_innerMtoF_buffer = this->mtof_tilde_01_innerMtoF_buffer->allocateIfNeeded();
    }
    
    number param_13_value_constrain(number v) const {
        v = (v > 10 ? 10 : (v < 1 ? 1 : v));
        return v;
    }
    
    void slide_tilde_03_x_set(number v) {
        this->slide_tilde_03_x = v;
    }
    
    number param_14_value_constrain(number v) const {
        v = (v > 20480 ? 20480 : (v < 20 ? 20 : v));
        return v;
    }
    
    void slide_tilde_04_x_set(number v) {
        this->slide_tilde_04_x = v;
    }
    
    number param_15_value_constrain(number v) const {
        v = (v > 4 ? 4 : (v < 0.1 ? 0.1 : v));
        return v;
    }
    
    void slide_tilde_07_x_set(number v) {
        this->slide_tilde_07_x = v;
    }
    
    number param_16_value_constrain(number v) const {
        v = (v > 10 ? 10 : (v < 0.1 ? 0.1 : v));
        return v;
    }
    
    void slide_tilde_10_x_set(number v) {
        this->slide_tilde_10_x = v;
    }
    
    number param_17_value_constrain(number v) const {
        v = (v > 10 ? 10 : (v < 0 ? 0 : v));
        return v;
    }
    
    void slide_tilde_13_x_set(number v) {
        this->slide_tilde_13_x = v;
    }
    
    number param_18_value_constrain(number v) const {
        v = (v > 40 ? 40 : (v < 0 ? 0 : v));
        return v;
    }
    
    void slide_tilde_12_x_set(number v) {
        this->slide_tilde_12_x = v;
    }
    
    void p_08_lfotype_set(number v) {
        {
            this->p_08->setParameterValue(0, v, this->_currentTime);
        }
    }
    
    void numberobj_01_output_set(number v) {
        this->p_08_lfotype_set(v);
    }
    
    void numberobj_01_value_set(number v) {
        this->numberobj_01_value_setter(v);
        v = this->numberobj_01_value;
        number localvalue = v;
    
        if ((bool)(true) && localvalue < 0) {
            localvalue = 0;
        }
    
        if ((bool)(true) && localvalue > 4) {
            localvalue = 4;
        }
    
        if (this->numberobj_01_currentFormat != 6) {
            localvalue = trunc(localvalue);
        }
    
        this->numberobj_01_output_set(localvalue);
    }
    
    void p_08_jitter_set(number v) {
        {
            this->p_08->setParameterValue(2, v, this->_currentTime);
        }
    }
    
    void numberobj_02_output_set(number v) {
        this->p_08_jitter_set(v);
    }
    
    void numberobj_02_value_set(number v) {
        this->numberobj_02_value_setter(v);
        v = this->numberobj_02_value;
        number localvalue = v;
    
        if ((bool)(true) && localvalue < 0) {
            localvalue = 0;
        }
    
        if ((bool)(true) && localvalue > 100) {
            localvalue = 100;
        }
    
        if (this->numberobj_02_currentFormat != 6) {
            localvalue = trunc(localvalue);
        }
    
        this->numberobj_02_output_set(localvalue);
    }
    
    void p_08_smooth_set(number v) {
        {
            this->p_08->setParameterValue(3, v, this->_currentTime);
        }
    }
    
    void numberobj_03_output_set(number v) {
        this->p_08_smooth_set(v);
    }
    
    void numberobj_03_value_set(number v) {
        this->numberobj_03_value_setter(v);
        v = this->numberobj_03_value;
        number localvalue = v;
    
        if ((bool)(true) && localvalue < 0) {
            localvalue = 0;
        }
    
        if ((bool)(true) && localvalue > 1000) {
            localvalue = 1000;
        }
    
        if (this->numberobj_03_currentFormat != 6) {
            localvalue = trunc(localvalue);
        }
    
        this->numberobj_03_output_set(localvalue);
    }
    
    void p_08_pulsewidth_set(number v) {
        {
            this->p_08->setParameterValue(4, v, this->_currentTime);
        }
    }
    
    void numberobj_04_output_set(number v) {
        this->p_08_pulsewidth_set(v);
    }
    
    void numberobj_04_value_set(number v) {
        this->numberobj_04_value_setter(v);
        v = this->numberobj_04_value;
        number localvalue = v;
    
        if ((bool)(true) && localvalue < 0) {
            localvalue = 0;
        }
    
        if ((bool)(true) && localvalue > 1) {
            localvalue = 1;
        }
    
        if (this->numberobj_04_currentFormat != 6) {
            localvalue = trunc(localvalue);
        }
    
        this->numberobj_04_output_set(localvalue);
    }
    
    void p_05_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
        RNBO_UNUSED(port);
        RNBO_UNUSED(channel);
        RNBO_UNUSED(status);
        this->p_05->processMidiEvent(_currentTime, 0, data, length);
    }
    
    void notein_01_outchannel_set(number ) {}
    
    void notein_01_releasevelocity_set(number ) {}
    
    void ip_08_value_set(number v) {
        this->ip_08_value = v;
        this->ip_08_fillSigBuf();
        this->ip_08_lastValue = v;
    }
    
    void expr_17_out1_set(number v) {
        this->expr_17_out1 = v;
        this->ip_08_value_set(this->expr_17_out1);
    }
    
    void expr_17_in1_set(number in1) {
        this->expr_17_in1 = in1;
        this->expr_17_out1_set(this->expr_17_in1 > this->expr_17_in2);//#map:>_obj-2:1
    }
    
    void notein_01_velocity_set(number v) {
        this->expr_17_in1_set(v);
    }
    
    void mtof_tilde_01_midivalue_set(number v) {
        this->mtof_tilde_01_midivalue = v;
    }
    
    void notein_01_notenumber_set(number v) {
        this->mtof_tilde_01_midivalue_set(v);
    }
    
    void notein_01_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
        RNBO_UNUSED(length);
        RNBO_UNUSED(port);
    
        if (channel == this->notein_01_channel || this->notein_01_channel <= 0) {
            if (status == 144 || status == 128) {
                this->notein_01_outchannel_set(channel);
    
                if (status == 128) {
                    this->notein_01_releasevelocity_set(data[2]);
                    this->notein_01_velocity_set(0);
                } else {
                    this->notein_01_releasevelocity_set(0);
                    this->notein_01_velocity_set(data[2]);
                }
    
                this->notein_01_notenumber_set(data[1]);
            }
        }
    }
    
    void p_07_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
        RNBO_UNUSED(port);
        RNBO_UNUSED(channel);
        RNBO_UNUSED(status);
        this->p_07->processMidiEvent(_currentTime, 0, data, length);
    }
    
    void p_08_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
        RNBO_UNUSED(port);
        RNBO_UNUSED(channel);
        RNBO_UNUSED(status);
        this->p_08->processMidiEvent(_currentTime, 0, data, length);
    }
    
    void ctlin_13_outchannel_set(number ) {}
    
    void ctlin_13_outcontroller_set(number ) {}
    
    void fromnormalized_13_output_set(number v) {
        this->param_13_value_set(v);
    }
    
    void fromnormalized_13_input_set(number v) {
        this->fromnormalized_13_output_set(this->fromnormalized(0, v));
    }
    
    void expr_14_out1_set(number v) {
        this->expr_14_out1 = v;
        this->fromnormalized_13_input_set(this->expr_14_out1);
    }
    
    void expr_14_in1_set(number in1) {
        this->expr_14_in1 = in1;
        this->expr_14_out1_set(this->expr_14_in1 * this->expr_14_in2);//#map:expr_14:1
    }
    
    void ctlin_13_value_set(number v) {
        this->expr_14_in1_set(v);
    }
    
    void ctlin_13_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
        RNBO_UNUSED(length);
        RNBO_UNUSED(port);
    
        if (status == 0xB0 && (channel == this->ctlin_13_channel || this->ctlin_13_channel == -1) && (data[1] == this->ctlin_13_controller || this->ctlin_13_controller == -1)) {
            this->ctlin_13_outchannel_set(channel);
            this->ctlin_13_outcontroller_set(data[1]);
            this->ctlin_13_value_set(data[2]);
            this->ctlin_13_status = 0;
        }
    }
    
    void ctlin_14_outchannel_set(number ) {}
    
    void ctlin_14_outcontroller_set(number ) {}
    
    void fromnormalized_14_output_set(number v) {
        this->param_14_value_set(v);
    }
    
    void fromnormalized_14_input_set(number v) {
        this->fromnormalized_14_output_set(this->fromnormalized(1, v));
    }
    
    void expr_15_out1_set(number v) {
        this->expr_15_out1 = v;
        this->fromnormalized_14_input_set(this->expr_15_out1);
    }
    
    void expr_15_in1_set(number in1) {
        this->expr_15_in1 = in1;
        this->expr_15_out1_set(this->expr_15_in1 * this->expr_15_in2);//#map:expr_15:1
    }
    
    void ctlin_14_value_set(number v) {
        this->expr_15_in1_set(v);
    }
    
    void ctlin_14_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
        RNBO_UNUSED(length);
        RNBO_UNUSED(port);
    
        if (status == 0xB0 && (channel == this->ctlin_14_channel || this->ctlin_14_channel == -1) && (data[1] == this->ctlin_14_controller || this->ctlin_14_controller == -1)) {
            this->ctlin_14_outchannel_set(channel);
            this->ctlin_14_outcontroller_set(data[1]);
            this->ctlin_14_value_set(data[2]);
            this->ctlin_14_status = 0;
        }
    }
    
    void ctlin_15_outchannel_set(number ) {}
    
    void ctlin_15_outcontroller_set(number ) {}
    
    void fromnormalized_15_output_set(number v) {
        this->param_15_value_set(v);
    }
    
    void fromnormalized_15_input_set(number v) {
        this->fromnormalized_15_output_set(this->fromnormalized(2, v));
    }
    
    void expr_16_out1_set(number v) {
        this->expr_16_out1 = v;
        this->fromnormalized_15_input_set(this->expr_16_out1);
    }
    
    void expr_16_in1_set(number in1) {
        this->expr_16_in1 = in1;
        this->expr_16_out1_set(this->expr_16_in1 * this->expr_16_in2);//#map:expr_16:1
    }
    
    void ctlin_15_value_set(number v) {
        this->expr_16_in1_set(v);
    }
    
    void ctlin_15_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
        RNBO_UNUSED(length);
        RNBO_UNUSED(port);
    
        if (status == 0xB0 && (channel == this->ctlin_15_channel || this->ctlin_15_channel == -1) && (data[1] == this->ctlin_15_controller || this->ctlin_15_controller == -1)) {
            this->ctlin_15_outchannel_set(channel);
            this->ctlin_15_outcontroller_set(data[1]);
            this->ctlin_15_value_set(data[2]);
            this->ctlin_15_status = 0;
        }
    }
    
    void ctlin_16_outchannel_set(number ) {}
    
    void ctlin_16_outcontroller_set(number ) {}
    
    void fromnormalized_16_output_set(number v) {
        this->param_16_value_set(v);
    }
    
    void fromnormalized_16_input_set(number v) {
        this->fromnormalized_16_output_set(this->fromnormalized(3, v));
    }
    
    void expr_18_out1_set(number v) {
        this->expr_18_out1 = v;
        this->fromnormalized_16_input_set(this->expr_18_out1);
    }
    
    void expr_18_in1_set(number in1) {
        this->expr_18_in1 = in1;
        this->expr_18_out1_set(this->expr_18_in1 * this->expr_18_in2);//#map:expr_18:1
    }
    
    void ctlin_16_value_set(number v) {
        this->expr_18_in1_set(v);
    }
    
    void ctlin_16_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
        RNBO_UNUSED(length);
        RNBO_UNUSED(port);
    
        if (status == 0xB0 && (channel == this->ctlin_16_channel || this->ctlin_16_channel == -1) && (data[1] == this->ctlin_16_controller || this->ctlin_16_controller == -1)) {
            this->ctlin_16_outchannel_set(channel);
            this->ctlin_16_outcontroller_set(data[1]);
            this->ctlin_16_value_set(data[2]);
            this->ctlin_16_status = 0;
        }
    }
    
    void midiouthelper_midiout_set(number ) {}
    
    void mtof_tilde_01_perform(number midivalue, SampleValue * out, Index n) {
        auto __mtof_tilde_01_base = this->mtof_tilde_01_base;
    
        for (Index i = 0; i < n; i++) {
            out[(Index)i] = this->mtof_tilde_01_innerMtoF_next(midivalue, __mtof_tilde_01_base);
        }
    }
    
    void slide_tilde_03_perform(number x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_03_prev = this->slide_tilde_03_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(40)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(40)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x - __slide_tilde_03_prev;
            __slide_tilde_03_prev = __slide_tilde_03_prev + ((x > __slide_tilde_03_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_03_prev;
        }
    
        this->slide_tilde_03_prev = __slide_tilde_03_prev;
    }
    
    void slide_tilde_04_perform(number x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_04_prev = this->slide_tilde_04_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(40)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(40)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x - __slide_tilde_04_prev;
            __slide_tilde_04_prev = __slide_tilde_04_prev + ((x > __slide_tilde_04_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_04_prev;
        }
    
        this->slide_tilde_04_prev = __slide_tilde_04_prev;
    }
    
    void slide_tilde_07_perform(number x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_07_prev = this->slide_tilde_07_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(40)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(40)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x - __slide_tilde_07_prev;
            __slide_tilde_07_prev = __slide_tilde_07_prev + ((x > __slide_tilde_07_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_07_prev;
        }
    
        this->slide_tilde_07_prev = __slide_tilde_07_prev;
    }
    
    void filtercoeff_tilde_01_perform(
        const Sample * frequency,
        number gain,
        const Sample * q,
        SampleValue * out1,
        SampleValue * out2,
        SampleValue * out3,
        SampleValue * out4,
        SampleValue * out5,
        Index n
    ) {
        RNBO_UNUSED(gain);
        auto __filtercoeff_tilde_01_resamp_value = this->filtercoeff_tilde_01_resamp_value;
        auto __filtercoeff_tilde_01_resamp_counter = this->filtercoeff_tilde_01_resamp_counter;
        auto __filtercoeff_tilde_01_lb2 = this->filtercoeff_tilde_01_lb2;
        auto __filtercoeff_tilde_01_lb1 = this->filtercoeff_tilde_01_lb1;
        auto __filtercoeff_tilde_01_la2 = this->filtercoeff_tilde_01_la2;
        auto __filtercoeff_tilde_01_la1 = this->filtercoeff_tilde_01_la1;
        auto __filtercoeff_tilde_01_la0 = this->filtercoeff_tilde_01_la0;
        auto __filtercoeff_tilde_01_c_type = this->filtercoeff_tilde_01_c_type;
        auto __filtercoeff_tilde_01_type = this->filtercoeff_tilde_01_type;
        auto __filtercoeff_tilde_01_c_q = this->filtercoeff_tilde_01_c_q;
        auto __filtercoeff_tilde_01_c_gain = this->filtercoeff_tilde_01_c_gain;
        auto __filtercoeff_tilde_01_c_frequency = this->filtercoeff_tilde_01_c_frequency;
        number sr = this->sr;
        number halfsr = sr * 0.5;
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number __frequency = frequency[(Index)i];
            number __q = q[(Index)i];
    
            if (__frequency == __filtercoeff_tilde_01_c_frequency && 1 == __filtercoeff_tilde_01_c_gain && __q == __filtercoeff_tilde_01_c_q && __filtercoeff_tilde_01_type == __filtercoeff_tilde_01_c_type) {
                out1[(Index)i] = __filtercoeff_tilde_01_la0;
                out2[(Index)i] = __filtercoeff_tilde_01_la1;
                out3[(Index)i] = __filtercoeff_tilde_01_la2;
                out4[(Index)i] = __filtercoeff_tilde_01_lb1;
                out5[(Index)i] = __filtercoeff_tilde_01_lb2;
                continue;
            }
    
            if (__filtercoeff_tilde_01_resamp_counter > 0) {
                __filtercoeff_tilde_01_resamp_counter--;
                out1[(Index)i] = __filtercoeff_tilde_01_la0;
                out2[(Index)i] = __filtercoeff_tilde_01_la1;
                out3[(Index)i] = __filtercoeff_tilde_01_la2;
                out4[(Index)i] = __filtercoeff_tilde_01_lb1;
                out5[(Index)i] = __filtercoeff_tilde_01_lb2;
                continue;
            }
    
            __filtercoeff_tilde_01_resamp_counter = __filtercoeff_tilde_01_resamp_value;
            __filtercoeff_tilde_01_c_frequency = __frequency;
            __filtercoeff_tilde_01_c_gain = 1;
            __filtercoeff_tilde_01_c_q = __q;
            __filtercoeff_tilde_01_c_type = __filtercoeff_tilde_01_type;
    
            if (__q < 1e-9)
                __q = 1e-9;
    
            __frequency = (__frequency > halfsr ? halfsr : (__frequency < 1 ? 1 : __frequency));
            number omega = __frequency * ((number)6.283185307179586 / sr);
            number cs = rnbo_cos(omega);
            number sn = rnbo_sin(omega);
            number one_over_gain = (number)1 / (number)1;
            number one_over_q = (number)1 / __q;
            number alpha = sn * 0.5 * one_over_q;
            number beta;
            number A;
            number one_over_a;
            number b0;
            number b0g;
    
            switch (__filtercoeff_tilde_01_type) {
            case 5:
                {
                A = this->safesqrt(1);
                beta = this->safesqrt((A * A + 1.) * one_over_q - (A - 1.) * (A - 1.));
                b0 = (number)1 / (A + 1. + (A - 1.) * cs + beta * sn);
                break;
                }
            case 6:
                {
                A = this->safesqrt(1);
                beta = this->safesqrt((A * A + 1.) * one_over_q - (A - 1.) * (A - 1.));
                b0 = (number)1 / (A + 1. - (A - 1.) * cs + beta * sn);
                break;
                }
            case 4:
                {
                A = this->safesqrt(1);
                one_over_a = (A == 0 ? 0 : (number)1 / A);
                b0 = (number)1 / (1. + alpha * one_over_a);
                break;
                }
            case 9:
            case 10:
            case 11:
            case 13:
            case 14:
                {
                b0 = (number)1 / (1. + alpha);
                b0g = (number)1 / (one_over_gain + alpha * one_over_gain);
                break;
                }
            default:
                {
                b0 = (number)1 / (1. + alpha);
                break;
                }
            }
    
            switch (__filtercoeff_tilde_01_type) {
            case 0:
                {
                __filtercoeff_tilde_01_la0 = __filtercoeff_tilde_01_la2 = (1. - cs) * 0.5 * b0;
                __filtercoeff_tilde_01_la1 = (1. - cs) * b0;
                __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                break;
                }
            case 1:
                {
                __filtercoeff_tilde_01_la0 = __filtercoeff_tilde_01_la2 = (1. + cs) * 0.5 * b0;
                __filtercoeff_tilde_01_la1 = -(1. + cs) * b0;
                __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                break;
                }
            case 2:
                {
                __filtercoeff_tilde_01_la0 = alpha * b0;
                __filtercoeff_tilde_01_la1 = 0.;
                __filtercoeff_tilde_01_la2 = -alpha * b0;
                __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                break;
                }
            case 7:
                {
                __filtercoeff_tilde_01_la0 = alpha * __q * b0;
                __filtercoeff_tilde_01_la1 = 0.;
                __filtercoeff_tilde_01_la2 = -alpha * __q * b0;
                __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                break;
                }
            case 3:
                {
                __filtercoeff_tilde_01_la1 = __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                __filtercoeff_tilde_01_la0 = __filtercoeff_tilde_01_la2 = b0;
                break;
                }
            case 8:
                {
                __filtercoeff_tilde_01_la1 = __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = __filtercoeff_tilde_01_la0 = (1. - alpha) * b0;
                __filtercoeff_tilde_01_la2 = 1.0;
                break;
                }
            case 4:
                {
                __filtercoeff_tilde_01_la0 = (1. + alpha * A) * b0;
                __filtercoeff_tilde_01_la1 = __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_la2 = (1. - alpha * A) * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha * one_over_a) * b0;
                break;
                }
            case 5:
                {
                __filtercoeff_tilde_01_la0 = A * (A + 1. - (A - 1.) * cs + beta * sn) * b0;
                __filtercoeff_tilde_01_la1 = 2. * A * (A - 1 - (A + 1) * cs) * b0;
                __filtercoeff_tilde_01_la2 = A * (A + 1. - (A - 1.) * cs - beta * sn) * b0;
                __filtercoeff_tilde_01_lb1 = -2. * (A - 1. + (A + 1.) * cs) * b0;
                __filtercoeff_tilde_01_lb2 = (A + 1. + (A - 1.) * cs - beta * sn) * b0;
                break;
                }
            case 6:
                {
                __filtercoeff_tilde_01_la0 = A * (A + 1. + (A - 1.) * cs + beta * sn) * b0;
                __filtercoeff_tilde_01_la1 = -2. * A * (A - 1. + (A + 1.) * cs) * b0;
                __filtercoeff_tilde_01_la2 = A * (A + 1. + (A - 1.) * cs - beta * sn) * b0;
                __filtercoeff_tilde_01_lb1 = 2. * (A - 1. - (A + 1.) * cs) * b0;
                __filtercoeff_tilde_01_lb2 = (A + 1. - (A - 1.) * cs - beta * sn) * b0;
                break;
                }
            case 9:
                {
                b0g = (number)1 / (one_over_gain + alpha * one_over_gain);
                __filtercoeff_tilde_01_la0 = __filtercoeff_tilde_01_la2 = (1. - cs) * 0.5 * b0g;
                __filtercoeff_tilde_01_la1 = (1. - cs) * b0g;
                __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                break;
                }
            case 10:
                {
                b0g = (number)1 / (one_over_gain + alpha * one_over_gain);
                __filtercoeff_tilde_01_la0 = __filtercoeff_tilde_01_la2 = (1. + cs) * 0.5 * b0g;
                __filtercoeff_tilde_01_la1 = -(1. + cs) * b0g;
                __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                break;
                }
            case 11:
                {
                __filtercoeff_tilde_01_la0 = alpha * 1 * b0;
                __filtercoeff_tilde_01_la1 = 0.;
                __filtercoeff_tilde_01_la2 = -alpha * 1 * b0;
                __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                break;
                }
            case 13:
                {
                __filtercoeff_tilde_01_la0 = alpha * 1 * __q * b0;
                __filtercoeff_tilde_01_la1 = 0.;
                __filtercoeff_tilde_01_la2 = -alpha * 1 * __q * b0;
                __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                break;
                }
            case 12:
                {
                b0g = (number)1 / (one_over_gain + alpha * one_over_gain);
                __filtercoeff_tilde_01_la1 = __filtercoeff_tilde_01_lb1 = -2. * cs;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                __filtercoeff_tilde_01_la1 *= b0g;
                __filtercoeff_tilde_01_lb1 *= b0;
                __filtercoeff_tilde_01_la0 = b0g;
                __filtercoeff_tilde_01_la2 = b0g;
                break;
                }
            case 14:
                {
                b0g = (number)1 / (one_over_gain + alpha * one_over_gain);
                __filtercoeff_tilde_01_la0 = (1. - alpha) * b0g;
                __filtercoeff_tilde_01_la1 = -2. * cs * b0g;
                __filtercoeff_tilde_01_la2 = 1;
                __filtercoeff_tilde_01_lb1 = -2. * cs * b0;
                __filtercoeff_tilde_01_lb2 = (1. - alpha) * b0;
                break;
                }
            case 15:
                {
                __filtercoeff_tilde_01_la0 = 1;
                __filtercoeff_tilde_01_la1 = 0;
                __filtercoeff_tilde_01_la2 = 0;
                __filtercoeff_tilde_01_lb1 = 0;
                __filtercoeff_tilde_01_lb2 = 0;
                }
            default:
                {
                break;
                }
            }
    
            out1[(Index)i] = __filtercoeff_tilde_01_la0;
            out2[(Index)i] = __filtercoeff_tilde_01_la1;
            out3[(Index)i] = __filtercoeff_tilde_01_la2;
            out4[(Index)i] = __filtercoeff_tilde_01_lb1;
            out5[(Index)i] = __filtercoeff_tilde_01_lb2;
        }
    
        this->filtercoeff_tilde_01_c_frequency = __filtercoeff_tilde_01_c_frequency;
        this->filtercoeff_tilde_01_c_gain = __filtercoeff_tilde_01_c_gain;
        this->filtercoeff_tilde_01_c_q = __filtercoeff_tilde_01_c_q;
        this->filtercoeff_tilde_01_c_type = __filtercoeff_tilde_01_c_type;
        this->filtercoeff_tilde_01_la0 = __filtercoeff_tilde_01_la0;
        this->filtercoeff_tilde_01_la1 = __filtercoeff_tilde_01_la1;
        this->filtercoeff_tilde_01_la2 = __filtercoeff_tilde_01_la2;
        this->filtercoeff_tilde_01_lb1 = __filtercoeff_tilde_01_lb1;
        this->filtercoeff_tilde_01_lb2 = __filtercoeff_tilde_01_lb2;
        this->filtercoeff_tilde_01_resamp_counter = __filtercoeff_tilde_01_resamp_counter;
    }
    
    void slide_tilde_05_perform(const Sample * x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_05_prev = this->slide_tilde_05_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(2500)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(2500)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x[(Index)i] - __slide_tilde_05_prev;
            __slide_tilde_05_prev = __slide_tilde_05_prev + ((x[(Index)i] > __slide_tilde_05_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_05_prev;
        }
    
        this->slide_tilde_05_prev = __slide_tilde_05_prev;
    }
    
    void slide_tilde_06_perform(const Sample * x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_06_prev = this->slide_tilde_06_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(2500)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(2500)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x[(Index)i] - __slide_tilde_06_prev;
            __slide_tilde_06_prev = __slide_tilde_06_prev + ((x[(Index)i] > __slide_tilde_06_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_06_prev;
        }
    
        this->slide_tilde_06_prev = __slide_tilde_06_prev;
    }
    
    void slide_tilde_08_perform(const Sample * x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_08_prev = this->slide_tilde_08_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(2500)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(2500)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x[(Index)i] - __slide_tilde_08_prev;
            __slide_tilde_08_prev = __slide_tilde_08_prev + ((x[(Index)i] > __slide_tilde_08_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_08_prev;
        }
    
        this->slide_tilde_08_prev = __slide_tilde_08_prev;
    }
    
    void slide_tilde_09_perform(const Sample * x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_09_prev = this->slide_tilde_09_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(2500)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(2500)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x[(Index)i] - __slide_tilde_09_prev;
            __slide_tilde_09_prev = __slide_tilde_09_prev + ((x[(Index)i] > __slide_tilde_09_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_09_prev;
        }
    
        this->slide_tilde_09_prev = __slide_tilde_09_prev;
    }
    
    void slide_tilde_11_perform(const Sample * x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_11_prev = this->slide_tilde_11_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(2500)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(2500)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x[(Index)i] - __slide_tilde_11_prev;
            __slide_tilde_11_prev = __slide_tilde_11_prev + ((x[(Index)i] > __slide_tilde_11_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_11_prev;
        }
    
        this->slide_tilde_11_prev = __slide_tilde_11_prev;
    }
    
    void ip_08_perform(SampleValue * out, Index n) {
        auto __ip_08_lastValue = this->ip_08_lastValue;
        auto __ip_08_lastIndex = this->ip_08_lastIndex;
    
        for (Index i = 0; i < n; i++) {
            out[(Index)i] = ((SampleIndex)(i) >= __ip_08_lastIndex ? __ip_08_lastValue : this->ip_08_sigbuf[(Index)i]);
        }
    
        __ip_08_lastIndex = 0;
        this->ip_08_lastIndex = __ip_08_lastIndex;
    }
    
    void slide_tilde_10_perform(number x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_10_prev = this->slide_tilde_10_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(40)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(40)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x - __slide_tilde_10_prev;
            __slide_tilde_10_prev = __slide_tilde_10_prev + ((x > __slide_tilde_10_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_10_prev;
        }
    
        this->slide_tilde_10_prev = __slide_tilde_10_prev;
    }
    
    void slide_tilde_12_perform(number x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_12_prev = this->slide_tilde_12_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(500)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(500)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x - __slide_tilde_12_prev;
            __slide_tilde_12_prev = __slide_tilde_12_prev + ((x > __slide_tilde_12_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_12_prev;
        }
    
        this->slide_tilde_12_prev = __slide_tilde_12_prev;
    }
    
    void p_08_perform(const SampleValue * in1, SampleValue * out1, Index n) {
        ConstSampleArray<1> ins = {in1};
        SampleArray<1> outs = {out1};
        this->p_08->process(ins, 1, outs, 1, n);
    }
    
    void slide_tilde_13_perform(number x, number up, number down, SampleValue * out1, Index n) {
        RNBO_UNUSED(down);
        RNBO_UNUSED(up);
        auto __slide_tilde_13_prev = this->slide_tilde_13_prev;
        auto iup = this->safediv(1., this->maximum(1., rnbo_abs(500)));
        auto idown = this->safediv(1., this->maximum(1., rnbo_abs(500)));
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number temp = x - __slide_tilde_13_prev;
            __slide_tilde_13_prev = __slide_tilde_13_prev + ((x > __slide_tilde_13_prev ? iup : idown)) * temp;
            out1[(Index)i] = __slide_tilde_13_prev;
        }
    
        this->slide_tilde_13_prev = __slide_tilde_13_prev;
    }
    
    void dspexpr_20_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
        }
    }
    
    void dspexpr_19_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            out1[(Index)i] = in1[(Index)i] + in2[(Index)i];//#map:_###_obj_###_:1
        }
    }
    
    void dspexpr_18_perform(const Sample * in1, number in2, number in3, SampleValue * out1, Index n) {
        RNBO_UNUSED(in3);
        RNBO_UNUSED(in2);
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            out1[(Index)i] = (in1[(Index)i] > 10 ? 10 : (in1[(Index)i] < 0.1 ? 0.1 : in1[(Index)i]));//#map:_###_obj_###_:1
        }
    }
    
    void signaladder_01_perform(
        const SampleValue * in1,
        const SampleValue * in2,
        SampleValue * out,
        Index n
    ) {
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            out[(Index)i] = in1[(Index)i] + in2[(Index)i];
        }
    }
    
    void p_06_perform(
        const SampleValue * in1,
        const SampleValue * in2,
        const SampleValue * in3,
        SampleValue * out1,
        SampleValue * out2,
        Index n
    ) {
        ConstSampleArray<3> ins = {in1, in2, in3};
        SampleArray<2> outs = {out1, out2};
        this->p_06->process(ins, 3, outs, 2, n);
    }
    
    void p_05_perform(
        const SampleValue * in1,
        const SampleValue * in2,
        SampleValue * out1,
        SampleValue * out2,
        Index n
    ) {
        ConstSampleArray<2> ins = {in1, in2};
        SampleArray<2> outs = {out1, out2};
        this->p_05->process(ins, 2, outs, 2, n);
    }
    
    void biquad_tilde_01_perform(
        const Sample * x,
        const Sample * a0,
        const Sample * a1,
        const Sample * a2,
        const Sample * b1,
        const Sample * b2,
        SampleValue * out1,
        Index n
    ) {
        auto __biquad_tilde_01_y2 = this->biquad_tilde_01_y2;
        auto __biquad_tilde_01_y1 = this->biquad_tilde_01_y1;
        auto __biquad_tilde_01_x2 = this->biquad_tilde_01_x2;
        auto __biquad_tilde_01_x1 = this->biquad_tilde_01_x1;
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            number tmp = x[(Index)i] * a0[(Index)i] + __biquad_tilde_01_x1 * a1[(Index)i] + __biquad_tilde_01_x2 * a2[(Index)i] - (__biquad_tilde_01_y1 * b1[(Index)i] + __biquad_tilde_01_y2 * b2[(Index)i]);
            __biquad_tilde_01_x2 = __biquad_tilde_01_x1;
            __biquad_tilde_01_x1 = x[(Index)i];
            __biquad_tilde_01_y2 = __biquad_tilde_01_y1;
            __biquad_tilde_01_y1 = tmp;
            out1[(Index)i] = tmp;
        }
    
        this->biquad_tilde_01_x1 = __biquad_tilde_01_x1;
        this->biquad_tilde_01_x2 = __biquad_tilde_01_x2;
        this->biquad_tilde_01_y1 = __biquad_tilde_01_y1;
        this->biquad_tilde_01_y2 = __biquad_tilde_01_y2;
    }
    
    void p_07_perform(const SampleValue * in1, SampleValue * out1, SampleValue * out2, Index n) {
        // subpatcher: stereo_delay
        ConstSampleArray<1> ins = {in1};
    
        SampleArray<2> outs = {out1, out2};
        this->p_07->process(ins, 1, outs, 2, n);
    }
    
    void signaladder_02_perform(
        const SampleValue * in1,
        const SampleValue * in2,
        SampleValue * out,
        Index n
    ) {
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            out[(Index)i] = in1[(Index)i] + in2[(Index)i];
        }
    }
    
    void signaladder_03_perform(
        const SampleValue * in1,
        const SampleValue * in2,
        SampleValue * out,
        Index n
    ) {
        Index i;
    
        for (i = 0; i < (Index)n; i++) {
            out[(Index)i] = in1[(Index)i] + in2[(Index)i];
        }
    }
    
    void stackprotect_perform(Index n) {
        RNBO_UNUSED(n);
        auto __stackprotect_count = this->stackprotect_count;
        __stackprotect_count = 0;
        this->stackprotect_count = __stackprotect_count;
    }
    
    void numberobj_01_value_setter(number v) {
        number localvalue = v;
    
        if ((bool)(true) && localvalue < 0) {
            localvalue = 0;
        }
    
        if ((bool)(true) && localvalue > 4) {
            localvalue = 4;
        }
    
        if (this->numberobj_01_currentFormat != 6) {
            localvalue = trunc(localvalue);
        }
    
        this->numberobj_01_value = localvalue;
    }
    
    void numberobj_02_value_setter(number v) {
        number localvalue = v;
    
        if ((bool)(true) && localvalue < 0) {
            localvalue = 0;
        }
    
        if ((bool)(true) && localvalue > 100) {
            localvalue = 100;
        }
    
        if (this->numberobj_02_currentFormat != 6) {
            localvalue = trunc(localvalue);
        }
    
        this->numberobj_02_value = localvalue;
    }
    
    void numberobj_03_value_setter(number v) {
        number localvalue = v;
    
        if ((bool)(true) && localvalue < 0) {
            localvalue = 0;
        }
    
        if ((bool)(true) && localvalue > 1000) {
            localvalue = 1000;
        }
    
        if (this->numberobj_03_currentFormat != 6) {
            localvalue = trunc(localvalue);
        }
    
        this->numberobj_03_value = localvalue;
    }
    
    void numberobj_04_value_setter(number v) {
        number localvalue = v;
    
        if ((bool)(true) && localvalue < 0) {
            localvalue = 0;
        }
    
        if ((bool)(true) && localvalue > 1) {
            localvalue = 1;
        }
    
        if (this->numberobj_04_currentFormat != 6) {
            localvalue = trunc(localvalue);
        }
    
        this->numberobj_04_value = localvalue;
    }
    
    number mtof_tilde_01_innerMtoF_next(number midivalue, number tuning) {
        if (midivalue == this->mtof_tilde_01_innerMtoF_lastInValue && tuning == this->mtof_tilde_01_innerMtoF_lastTuning)
            return this->mtof_tilde_01_innerMtoF_lastOutValue;
    
        this->mtof_tilde_01_innerMtoF_lastInValue = midivalue;
        this->mtof_tilde_01_innerMtoF_lastTuning = tuning;
        number result = 0;
    
        {
            result = rnbo_exp(.057762265 * (midivalue - 69.0));
        }
    
        this->mtof_tilde_01_innerMtoF_lastOutValue = tuning * result;
        return this->mtof_tilde_01_innerMtoF_lastOutValue;
    }
    
    void mtof_tilde_01_innerMtoF_reset() {
        this->mtof_tilde_01_innerMtoF_lastInValue = 0;
        this->mtof_tilde_01_innerMtoF_lastOutValue = 0;
        this->mtof_tilde_01_innerMtoF_lastTuning = 0;
    }
    
    void mtof_tilde_01_innerScala_mid(Int v) {
        this->mtof_tilde_01_innerScala_kbmMid = v;
        this->mtof_tilde_01_innerScala_updateRefFreq();
    }
    
    void mtof_tilde_01_innerScala_ref(Int v) {
        this->mtof_tilde_01_innerScala_kbmRefNum = v;
        this->mtof_tilde_01_innerScala_updateRefFreq();
    }
    
    void mtof_tilde_01_innerScala_base(number v) {
        this->mtof_tilde_01_innerScala_kbmRefFreq = v;
        this->mtof_tilde_01_innerScala_updateRefFreq();
    }
    
    void mtof_tilde_01_innerScala_init() {
        list sclValid = {
            12,
            100,
            0,
            200,
            0,
            300,
            0,
            400,
            0,
            500,
            0,
            600,
            0,
            700,
            0,
            800,
            0,
            900,
            0,
            1000,
            0,
            1100,
            0,
            2,
            1
        };
    
        this->mtof_tilde_01_innerScala_updateScale(sclValid);
    }
    
    template<typename LISTTYPE = list> void mtof_tilde_01_innerScala_update(const LISTTYPE& scale, const LISTTYPE& map) {
        if (scale->length > 0) {
            this->mtof_tilde_01_innerScala_updateScale(scale);
        }
    
        if (map->length > 0) {
            this->mtof_tilde_01_innerScala_updateMap(map);
        }
    }
    
    number mtof_tilde_01_innerScala_mtof(number note) {
        if ((bool)(this->mtof_tilde_01_innerScala_lastValid) && this->mtof_tilde_01_innerScala_lastNote == note) {
            return this->mtof_tilde_01_innerScala_lastFreq;
        }
    
        array<Int, 2> degoct = this->mtof_tilde_01_innerScala_applyKBM(note);
        number out = 0;
    
        if (degoct[1] > 0) {
            out = this->mtof_tilde_01_innerScala_applySCL(degoct[0], fract(note), this->mtof_tilde_01_innerScala_refFreq);
        }
    
        this->mtof_tilde_01_innerScala_updateLast(note, out);
        return out;
    }
    
    number mtof_tilde_01_innerScala_ftom(number hz) {
        if (hz <= 0.0) {
            return 0.0;
        }
    
        if ((bool)(this->mtof_tilde_01_innerScala_lastValid) && this->mtof_tilde_01_innerScala_lastFreq == hz) {
            return this->mtof_tilde_01_innerScala_lastNote;
        }
    
        array<number, 2> df = this->mtof_tilde_01_innerScala_hztodeg(hz);
        Int degree = (Int)(df[0]);
        number frac = df[1];
        number out = 0;
    
        if (this->mtof_tilde_01_innerScala_kbmSize == 0) {
            out = this->mtof_tilde_01_innerScala_kbmMid + degree;
        } else {
            array<Int, 2> octdeg = this->mtof_tilde_01_innerScala_octdegree(degree, this->mtof_tilde_01_innerScala_kbmOctaveDegree);
            number oct = (number)(octdeg[0]);
            Int index = (Int)(octdeg[1]);
            Index entry = 0;
    
            for (Index i = 0; i < this->mtof_tilde_01_innerScala_kbmMapSize; i++) {
                if (index == this->mtof_tilde_01_innerScala_kbmValid[(Index)(i + this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET)]) {
                    entry = i;
                    break;
                }
            }
    
            out = oct * this->mtof_tilde_01_innerScala_kbmSize + entry + this->mtof_tilde_01_innerScala_kbmMid;
        }
    
        out = out + frac;
        this->mtof_tilde_01_innerScala_updateLast(out, hz);
        return this->mtof_tilde_01_innerScala_lastNote;
    }
    
    template<typename LISTTYPE = list> Int mtof_tilde_01_innerScala_updateScale(const LISTTYPE& scl) {
        if (scl->length < 1) {
            return 0;
        }
    
        number sclDataEntries = scl[0] * 2 + 1;
    
        if (sclDataEntries <= scl->length) {
            this->mtof_tilde_01_innerScala_lastValid = false;
            this->mtof_tilde_01_innerScala_sclExpMul = {};
            number last = 1;
    
            for (Index i = 1; i < sclDataEntries; i += 2) {
                const number c = (const number)(scl[(Index)(i + 0)]);
                const number d = (const number)(scl[(Index)(i + 1)]);
    
                if (d <= 0) {
                    last = c / (number)1200;
                } else {
                    last = rnbo_log2(c / d);
                }
    
                this->mtof_tilde_01_innerScala_sclExpMul->push(last);
            }
    
            this->mtof_tilde_01_innerScala_sclOctaveMul = last;
            this->mtof_tilde_01_innerScala_sclEntryCount = (Int)(this->mtof_tilde_01_innerScala_sclExpMul->length);
    
            if (scl->length >= sclDataEntries + 3) {
                this->mtof_tilde_01_innerScala_kbmMid = (Int)(scl[(Index)(sclDataEntries + 2)]);
                this->mtof_tilde_01_innerScala_kbmRefNum = (Int)(scl[(Index)(sclDataEntries + 1)]);
                this->mtof_tilde_01_innerScala_kbmRefFreq = scl[(Index)(sclDataEntries + 0)];
                this->mtof_tilde_01_innerScala_kbmSize = (Int)(0);
            }
    
            this->mtof_tilde_01_innerScala_updateRefFreq();
            return 1;
        }
    
        return 0;
    }
    
    template<typename LISTTYPE = list> Int mtof_tilde_01_innerScala_updateMap(const LISTTYPE& kbm) {
        list _kbm = kbm;
    
        if (_kbm->length == 1 && _kbm[0] == 0.0) {
            _kbm = {0.0, 0.0, 0.0, 60.0, 69.0, 440.0};
        }
    
        if (_kbm->length >= 6 && _kbm[0] >= 0.0) {
            this->mtof_tilde_01_innerScala_lastValid = false;
            Index size = (Index)(_kbm[0]);
            Int octave = 12;
    
            if (_kbm->length > 6) {
                octave = (Int)(_kbm[6]);
            }
    
            if (size > 0 && _kbm->length < this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET) {
                return 0;
            }
    
            this->mtof_tilde_01_innerScala_kbmSize = (Int)(size);
            this->mtof_tilde_01_innerScala_kbmMin = (Int)(_kbm[1]);
            this->mtof_tilde_01_innerScala_kbmMax = (Int)(_kbm[2]);
            this->mtof_tilde_01_innerScala_kbmMid = (Int)(_kbm[3]);
            this->mtof_tilde_01_innerScala_kbmRefNum = (Int)(_kbm[4]);
            this->mtof_tilde_01_innerScala_kbmRefFreq = _kbm[5];
            this->mtof_tilde_01_innerScala_kbmOctaveDegree = octave;
            this->mtof_tilde_01_innerScala_kbmValid = _kbm;
            this->mtof_tilde_01_innerScala_kbmMapSize = (_kbm->length - this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET > _kbm->length ? _kbm->length : (_kbm->length - this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET < 0 ? 0 : _kbm->length - this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET));
            this->mtof_tilde_01_innerScala_updateRefFreq();
            return 1;
        }
    
        return 0;
    }
    
    void mtof_tilde_01_innerScala_updateLast(number note, number freq) {
        this->mtof_tilde_01_innerScala_lastValid = true;
        this->mtof_tilde_01_innerScala_lastNote = note;
        this->mtof_tilde_01_innerScala_lastFreq = freq;
    }
    
    array<number, 2> mtof_tilde_01_innerScala_hztodeg(number hz) {
        number hza = rnbo_abs(hz);
    
        number octave = rnbo_floor(
            rnbo_log2(hza / this->mtof_tilde_01_innerScala_refFreq) / this->mtof_tilde_01_innerScala_sclOctaveMul
        );
    
        Int i = 0;
        number frac = 0;
        number n = 0;
    
        for (; i < this->mtof_tilde_01_innerScala_sclEntryCount; i++) {
            number c = this->mtof_tilde_01_innerScala_applySCLOctIndex(octave, i + 0, 0.0, this->mtof_tilde_01_innerScala_refFreq);
            n = this->mtof_tilde_01_innerScala_applySCLOctIndex(octave, i + 1, 0.0, this->mtof_tilde_01_innerScala_refFreq);
    
            if (c <= hza && hza < n) {
                if (c != hza) {
                    frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
                }
    
                break;
            }
        }
    
        if (i == this->mtof_tilde_01_innerScala_sclEntryCount && n != hza) {
            number c = n;
            n = this->mtof_tilde_01_innerScala_applySCLOctIndex(octave + 1, 0, 0.0, this->mtof_tilde_01_innerScala_refFreq);
            frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
        }
    
        number deg = i + octave * this->mtof_tilde_01_innerScala_sclEntryCount;
    
        {
            deg = rnbo_fround((deg + frac) * 1 / (number)1) * 1;
            frac = 0.0;
        }
    
        return {deg, frac};
    }
    
    array<Int, 2> mtof_tilde_01_innerScala_octdegree(Int degree, Int count) {
        Int octave = 0;
        Int index = 0;
    
        if (degree < 0) {
            octave = -(1 + (-1 - degree) / count);
            index = -degree % count;
    
            if (index > 0) {
                index = count - index;
            }
        } else {
            octave = degree / count;
            index = degree % count;
        }
    
        return {octave, index};
    }
    
    array<Int, 2> mtof_tilde_01_innerScala_applyKBM(number note) {
        if ((this->mtof_tilde_01_innerScala_kbmMin == this->mtof_tilde_01_innerScala_kbmMax && this->mtof_tilde_01_innerScala_kbmMax == 0) || (note >= this->mtof_tilde_01_innerScala_kbmMin && note <= this->mtof_tilde_01_innerScala_kbmMax)) {
            Int degree = (Int)(rnbo_floor(note - this->mtof_tilde_01_innerScala_kbmMid));
    
            if (this->mtof_tilde_01_innerScala_kbmSize == 0) {
                return {degree, 1};
            }
    
            array<Int, 2> octdeg = this->mtof_tilde_01_innerScala_octdegree(degree, this->mtof_tilde_01_innerScala_kbmSize);
            Int octave = (Int)(octdeg[0]);
            Index index = (Index)(octdeg[1]);
    
            if (this->mtof_tilde_01_innerScala_kbmMapSize > index) {
                degree = (Int)(this->mtof_tilde_01_innerScala_kbmValid[(Index)(this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET + index)]);
    
                if (degree >= 0) {
                    return {degree + octave * this->mtof_tilde_01_innerScala_kbmOctaveDegree, 1};
                }
            }
        }
    
        return {-1, 0};
    }
    
    number mtof_tilde_01_innerScala_applySCL(Int degree, number frac, number refFreq) {
        array<Int, 2> octdeg = this->mtof_tilde_01_innerScala_octdegree(degree, this->mtof_tilde_01_innerScala_sclEntryCount);
        return this->mtof_tilde_01_innerScala_applySCLOctIndex(octdeg[0], octdeg[1], frac, refFreq);
    }
    
    number mtof_tilde_01_innerScala_applySCLOctIndex(number octave, Int index, number frac, number refFreq) {
        number p = 0;
    
        if (index > 0) {
            p = this->mtof_tilde_01_innerScala_sclExpMul[(Index)(index - 1)];
        }
    
        if (frac > 0) {
            p = this->linearinterp(frac, p, this->mtof_tilde_01_innerScala_sclExpMul[(Index)index]);
        } else if (frac < 0) {
            p = this->linearinterp(-frac, this->mtof_tilde_01_innerScala_sclExpMul[(Index)index], p);
        }
    
        return refFreq * rnbo_pow(2, p + octave * this->mtof_tilde_01_innerScala_sclOctaveMul);
    }
    
    void mtof_tilde_01_innerScala_updateRefFreq() {
        this->mtof_tilde_01_innerScala_lastValid = false;
        Int refOffset = (Int)(this->mtof_tilde_01_innerScala_kbmRefNum - this->mtof_tilde_01_innerScala_kbmMid);
    
        if (refOffset == 0) {
            this->mtof_tilde_01_innerScala_refFreq = this->mtof_tilde_01_innerScala_kbmRefFreq;
        } else {
            Int base = (Int)(this->mtof_tilde_01_innerScala_kbmSize);
    
            if (base < 1) {
                base = this->mtof_tilde_01_innerScala_sclEntryCount;
            }
    
            array<Int, 2> octdeg = this->mtof_tilde_01_innerScala_octdegree(refOffset, base);
            number oct = (number)(octdeg[0]);
            Int index = (Int)(octdeg[1]);
    
            if (base > 0) {
                oct = oct + rnbo_floor(index / base);
                index = index % base;
            }
    
            if (index >= 0 && index < this->mtof_tilde_01_innerScala_kbmSize) {
                if (index < (Int)(this->mtof_tilde_01_innerScala_kbmMapSize)) {
                    index = (Int)(this->mtof_tilde_01_innerScala_kbmValid[(Index)((Index)(index) + this->mtof_tilde_01_innerScala_KBM_MAP_OFFSET)]);
                } else {
                    index = -1;
                }
            }
    
            if (index < 0 || index > (Int)(this->mtof_tilde_01_innerScala_sclExpMul->length))
                {} else {
                number p = 0;
    
                if (index > 0) {
                    p = this->mtof_tilde_01_innerScala_sclExpMul[(Index)(index - 1)];
                }
    
                this->mtof_tilde_01_innerScala_refFreq = this->mtof_tilde_01_innerScala_kbmRefFreq / rnbo_pow(2, p + oct * this->mtof_tilde_01_innerScala_sclOctaveMul);
            }
        }
    }
    
    void mtof_tilde_01_innerScala_reset() {
        this->mtof_tilde_01_innerScala_lastValid = false;
        this->mtof_tilde_01_innerScala_lastNote = 0;
        this->mtof_tilde_01_innerScala_lastFreq = 0;
        this->mtof_tilde_01_innerScala_sclEntryCount = 0;
        this->mtof_tilde_01_innerScala_sclOctaveMul = 1;
        this->mtof_tilde_01_innerScala_sclExpMul = {};
        this->mtof_tilde_01_innerScala_kbmValid = {0, 0, 0, 60, 69, 440};
        this->mtof_tilde_01_innerScala_kbmMid = 60;
        this->mtof_tilde_01_innerScala_kbmRefNum = 69;
        this->mtof_tilde_01_innerScala_kbmRefFreq = 440;
        this->mtof_tilde_01_innerScala_kbmSize = 0;
        this->mtof_tilde_01_innerScala_kbmMin = 0;
        this->mtof_tilde_01_innerScala_kbmMax = 0;
        this->mtof_tilde_01_innerScala_kbmOctaveDegree = 12;
        this->mtof_tilde_01_innerScala_kbmMapSize = 0;
        this->mtof_tilde_01_innerScala_refFreq = 261.63;
    }
    
    void mtof_tilde_01_init() {
        this->mtof_tilde_01_innerScala_update(this->mtof_tilde_01_scale, this->mtof_tilde_01_map);
    }
    
    void biquad_tilde_01_reset() {
        this->biquad_tilde_01_x1 = 0;
        this->biquad_tilde_01_x2 = 0;
        this->biquad_tilde_01_y1 = 0;
        this->biquad_tilde_01_y2 = 0;
    }
    
    void biquad_tilde_01_dspsetup(bool force) {
        if ((bool)(this->biquad_tilde_01_setupDone) && (bool)(!(bool)(force)))
            return;
    
        this->biquad_tilde_01_reset();
        this->biquad_tilde_01_setupDone = true;
    }
    
    void param_13_getPresetValue(PatcherStateInterface& preset) {
        preset["value"] = this->param_13_value;
    }
    
    void param_13_setPresetValue(PatcherStateInterface& preset) {
        if ((bool)(stateIsEmpty(preset)))
            return;
    
        this->param_13_value_set(preset["value"]);
    }
    
    void filtercoeff_tilde_01_dspsetup(bool force) {
        if ((bool)(this->filtercoeff_tilde_01_setupDone) && (bool)(!(bool)(force)))
            return;
    
        this->filtercoeff_tilde_01_resamp_value = this->vs;
        this->filtercoeff_tilde_01_setupDone = true;
    }
    
    void param_14_getPresetValue(PatcherStateInterface& preset) {
        preset["value"] = this->param_14_value;
    }
    
    void param_14_setPresetValue(PatcherStateInterface& preset) {
        if ((bool)(stateIsEmpty(preset)))
            return;
    
        this->param_14_value_set(preset["value"]);
    }
    
    void param_15_getPresetValue(PatcherStateInterface& preset) {
        preset["value"] = this->param_15_value;
    }
    
    void param_15_setPresetValue(PatcherStateInterface& preset) {
        if ((bool)(stateIsEmpty(preset)))
            return;
    
        this->param_15_value_set(preset["value"]);
    }
    
    void ip_08_init() {
        this->ip_08_lastValue = this->ip_08_value;
    }
    
    void ip_08_fillSigBuf() {
        if ((bool)(this->ip_08_sigbuf)) {
            SampleIndex k = (SampleIndex)(this->sampleOffsetIntoNextAudioBuffer);
    
            if (k >= (SampleIndex)(this->vs))
                k = (SampleIndex)(this->vs) - 1;
    
            for (SampleIndex i = (SampleIndex)(this->ip_08_lastIndex); i < k; i++) {
                if (this->ip_08_resetCount > 0) {
                    this->ip_08_sigbuf[(Index)i] = 1;
                    this->ip_08_resetCount--;
                } else {
                    this->ip_08_sigbuf[(Index)i] = this->ip_08_lastValue;
                }
            }
    
            this->ip_08_lastIndex = k;
        }
    }
    
    void ip_08_dspsetup(bool force) {
        if ((bool)(this->ip_08_setupDone) && (bool)(!(bool)(force)))
            return;
    
        this->ip_08_lastIndex = 0;
        this->ip_08_setupDone = true;
    }
    
    void param_16_getPresetValue(PatcherStateInterface& preset) {
        preset["value"] = this->param_16_value;
    }
    
    void param_16_setPresetValue(PatcherStateInterface& preset) {
        if ((bool)(stateIsEmpty(preset)))
            return;
    
        this->param_16_value_set(preset["value"]);
    }
    
    void param_17_getPresetValue(PatcherStateInterface& preset) {
        preset["value"] = this->param_17_value;
    }
    
    void param_17_setPresetValue(PatcherStateInterface& preset) {
        if ((bool)(stateIsEmpty(preset)))
            return;
    
        this->param_17_value_set(preset["value"]);
    }
    
    void param_18_getPresetValue(PatcherStateInterface& preset) {
        preset["value"] = this->param_18_value;
    }
    
    void param_18_setPresetValue(PatcherStateInterface& preset) {
        if ((bool)(stateIsEmpty(preset)))
            return;
    
        this->param_18_value_set(preset["value"]);
    }
    
    void numberobj_01_init() {
        this->numberobj_01_currentFormat = 6;
    }
    
    void numberobj_01_getPresetValue(PatcherStateInterface& preset) {
        preset["value"] = this->numberobj_01_value;
    }
    
    void numberobj_01_setPresetValue(PatcherStateInterface& preset) {
        if ((bool)(stateIsEmpty(preset)))
            return;
    
        this->numberobj_01_value_set(preset["value"]);
    }
    
    void numberobj_02_init() {
        this->numberobj_02_currentFormat = 6;
    }
    
    void numberobj_02_getPresetValue(PatcherStateInterface& preset) {
        preset["value"] = this->numberobj_02_value;
    }
    
    void numberobj_02_setPresetValue(PatcherStateInterface& preset) {
        if ((bool)(stateIsEmpty(preset)))
            return;
    
        this->numberobj_02_value_set(preset["value"]);
    }
    
    void numberobj_03_init() {
        this->numberobj_03_currentFormat = 6;
    }
    
    void numberobj_03_getPresetValue(PatcherStateInterface& preset) {
        preset["value"] = this->numberobj_03_value;
    }
    
    void numberobj_03_setPresetValue(PatcherStateInterface& preset) {
        if ((bool)(stateIsEmpty(preset)))
            return;
    
        this->numberobj_03_value_set(preset["value"]);
    }
    
    void numberobj_04_init() {
        this->numberobj_04_currentFormat = 6;
    }
    
    void numberobj_04_getPresetValue(PatcherStateInterface& preset) {
        preset["value"] = this->numberobj_04_value;
    }
    
    void numberobj_04_setPresetValue(PatcherStateInterface& preset) {
        if ((bool)(stateIsEmpty(preset)))
            return;
    
        this->numberobj_04_value_set(preset["value"]);
    }
    
    void midiouthelper_sendMidi(number v) {
        this->midiouthelper_midiout_set(v);
    }
    
    bool stackprotect_check() {
        this->stackprotect_count++;
    
        if (this->stackprotect_count > 128) {
            console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
            return true;
        }
    
        return false;
    }
    
    Index getPatcherSerial() const {
        return 0;
    }
    
    void sendParameter(ParameterIndex index, bool ignoreValue) {
        if (this->_voiceIndex == 1)
            this->getPatcher()->sendParameter(index + this->parameterOffset, ignoreValue);
    }
    
    void scheduleParamInit(ParameterIndex index, Index order) {
        this->getPatcher()->scheduleParamInit(index + this->parameterOffset, order);
    }
    
    void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
        RNBO_UNUSED(inProcess);
        RNBO_UNUSED(engine);
        this->_currentTime = time;
        auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));
    
        if (offset >= (SampleIndex)(this->vs))
            offset = (SampleIndex)(this->vs) - 1;
    
        if (offset < 0)
            offset = 0;
    
        this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
    }
    
    void assign_defaults()
    {
        p_05_target = 0;
        p_06_target = 0;
        notein_01_channel = 0;
        mtof_tilde_01_midivalue = 0;
        mtof_tilde_01_base = 440;
        biquad_tilde_01_x = 0;
        biquad_tilde_01_a0 = 0;
        biquad_tilde_01_a1 = 0;
        biquad_tilde_01_a2 = 0;
        biquad_tilde_01_b1 = 0;
        biquad_tilde_01_b2 = 0;
        p_07_target = 0;
        slide_tilde_03_x = 0;
        slide_tilde_03_up = 40;
        slide_tilde_03_down = 40;
        param_13_value = 0.1;
        slide_tilde_04_x = 0;
        slide_tilde_04_up = 40;
        slide_tilde_04_down = 40;
        slide_tilde_05_x = 0;
        slide_tilde_05_up = 2500;
        slide_tilde_05_down = 2500;
        filtercoeff_tilde_01_frequency = 1000;
        filtercoeff_tilde_01_gain = 1;
        filtercoeff_tilde_01_q = 1;
        filtercoeff_tilde_01_type = 0;
        param_14_value = 12000;
        slide_tilde_06_x = 0;
        slide_tilde_06_up = 2500;
        slide_tilde_06_down = 2500;
        slide_tilde_07_x = 0;
        slide_tilde_07_up = 40;
        slide_tilde_07_down = 40;
        param_15_value = 1;
        ip_08_value = 0;
        ip_08_impulse = 0;
        expr_17_in1 = 0;
        expr_17_in2 = 0;
        expr_17_out1 = 0;
        slide_tilde_08_x = 0;
        slide_tilde_08_up = 2500;
        slide_tilde_08_down = 2500;
        slide_tilde_09_x = 0;
        slide_tilde_09_up = 2500;
        slide_tilde_09_down = 2500;
        dspexpr_18_in1 = 0;
        dspexpr_18_in2 = 0.1;
        dspexpr_18_in3 = 10;
        dspexpr_19_in1 = 0;
        dspexpr_19_in2 = 0;
        slide_tilde_10_x = 0;
        slide_tilde_10_up = 40;
        slide_tilde_10_down = 40;
        param_16_value = 1;
        slide_tilde_11_x = 0;
        slide_tilde_11_up = 2500;
        slide_tilde_11_down = 2500;
        dspexpr_20_in1 = 0;
        dspexpr_20_in2 = 10;
        slide_tilde_12_x = 0;
        slide_tilde_12_up = 500;
        slide_tilde_12_down = 500;
        p_08_target = 0;
        slide_tilde_13_x = 0;
        slide_tilde_13_up = 500;
        slide_tilde_13_down = 500;
        param_17_value = 0;
        param_18_value = 0.2;
        numberobj_01_value = 0;
        numberobj_01_value_setter(numberobj_01_value);
        numberobj_02_value = 0;
        numberobj_02_value_setter(numberobj_02_value);
        numberobj_03_value = 0;
        numberobj_03_value_setter(numberobj_03_value);
        numberobj_04_value = 0;
        numberobj_04_value_setter(numberobj_04_value);
        ctlin_13_input = 0;
        ctlin_13_controller = 0;
        ctlin_13_channel = -1;
        expr_14_in1 = 0;
        expr_14_in2 = 0.007874015748;
        expr_14_out1 = 0;
        ctlin_14_input = 0;
        ctlin_14_controller = 0;
        ctlin_14_channel = -1;
        expr_15_in1 = 0;
        expr_15_in2 = 0.007874015748;
        expr_15_out1 = 0;
        ctlin_15_input = 0;
        ctlin_15_controller = 0;
        ctlin_15_channel = -1;
        expr_16_in1 = 0;
        expr_16_in2 = 0.007874015748;
        expr_16_out1 = 0;
        ctlin_16_input = 0;
        ctlin_16_controller = 0;
        ctlin_16_channel = -1;
        expr_18_in1 = 0;
        expr_18_in2 = 0.007874015748;
        expr_18_out1 = 0;
        _currentTime = 0;
        audioProcessSampleCount = 0;
        sampleOffsetIntoNextAudioBuffer = 0;
        zeroBuffer = nullptr;
        dummyBuffer = nullptr;
        signals[0] = nullptr;
        signals[1] = nullptr;
        signals[2] = nullptr;
        signals[3] = nullptr;
        signals[4] = nullptr;
        signals[5] = nullptr;
        signals[6] = nullptr;
        signals[7] = nullptr;
        signals[8] = nullptr;
        signals[9] = nullptr;
        signals[10] = nullptr;
        signals[11] = nullptr;
        didAllocateSignals = 0;
        vs = 0;
        maxvs = 0;
        sr = 44100;
        invsr = 0.000022675736961451248;
        notein_01_status = 0;
        notein_01_byte1 = -1;
        notein_01_inchan = 0;
        mtof_tilde_01_innerMtoF_lastInValue = 0;
        mtof_tilde_01_innerMtoF_lastOutValue = 0;
        mtof_tilde_01_innerMtoF_lastTuning = 0;
        mtof_tilde_01_innerScala_lastValid = false;
        mtof_tilde_01_innerScala_lastNote = 0;
        mtof_tilde_01_innerScala_lastFreq = 0;
        mtof_tilde_01_innerScala_sclEntryCount = 0;
        mtof_tilde_01_innerScala_sclOctaveMul = 1;
        mtof_tilde_01_innerScala_kbmValid = { 0, 0, 0, 60, 69, 440 };
        mtof_tilde_01_innerScala_kbmMid = 60;
        mtof_tilde_01_innerScala_kbmRefNum = 69;
        mtof_tilde_01_innerScala_kbmRefFreq = 440;
        mtof_tilde_01_innerScala_kbmSize = 0;
        mtof_tilde_01_innerScala_kbmMin = 0;
        mtof_tilde_01_innerScala_kbmMax = 0;
        mtof_tilde_01_innerScala_kbmOctaveDegree = 12;
        mtof_tilde_01_innerScala_kbmMapSize = 0;
        mtof_tilde_01_innerScala_refFreq = 261.63;
        biquad_tilde_01_x1 = 0;
        biquad_tilde_01_x2 = 0;
        biquad_tilde_01_y1 = 0;
        biquad_tilde_01_y2 = 0;
        biquad_tilde_01_setupDone = false;
        slide_tilde_03_prev = 0;
        param_13_lastValue = 0;
        slide_tilde_04_prev = 0;
        slide_tilde_05_prev = 0;
        filtercoeff_tilde_01_la0 = 0;
        filtercoeff_tilde_01_la1 = 0;
        filtercoeff_tilde_01_la2 = 0;
        filtercoeff_tilde_01_lb1 = 0;
        filtercoeff_tilde_01_lb2 = 0;
        filtercoeff_tilde_01_c_frequency = 0;
        filtercoeff_tilde_01_c_gain = 1;
        filtercoeff_tilde_01_c_q = 1;
        filtercoeff_tilde_01_c_type = 15;
        filtercoeff_tilde_01_resamp_counter = -1;
        filtercoeff_tilde_01_resamp_value = -1;
        filtercoeff_tilde_01_setupDone = false;
        param_14_lastValue = 0;
        slide_tilde_06_prev = 0;
        slide_tilde_07_prev = 0;
        param_15_lastValue = 0;
        ip_08_lastIndex = 0;
        ip_08_lastValue = 0;
        ip_08_resetCount = 0;
        ip_08_sigbuf = nullptr;
        ip_08_setupDone = false;
        slide_tilde_08_prev = 0;
        slide_tilde_09_prev = 0;
        slide_tilde_10_prev = 0;
        param_16_lastValue = 0;
        slide_tilde_11_prev = 0;
        slide_tilde_12_prev = 0;
        slide_tilde_13_prev = 0;
        param_17_lastValue = 0;
        param_18_lastValue = 0;
        numberobj_01_currentFormat = 6;
        numberobj_01_lastValue = 0;
        numberobj_02_currentFormat = 6;
        numberobj_02_lastValue = 0;
        numberobj_03_currentFormat = 6;
        numberobj_03_lastValue = 0;
        numberobj_04_currentFormat = 6;
        numberobj_04_lastValue = 0;
        ctlin_13_status = 0;
        ctlin_13_byte1 = -1;
        ctlin_13_inchan = 0;
        ctlin_14_status = 0;
        ctlin_14_byte1 = -1;
        ctlin_14_inchan = 0;
        ctlin_15_status = 0;
        ctlin_15_byte1 = -1;
        ctlin_15_inchan = 0;
        ctlin_16_status = 0;
        ctlin_16_byte1 = -1;
        ctlin_16_inchan = 0;
        stackprotect_count = 0;
        _voiceIndex = 0;
        _noteNumber = 0;
        isMuted = 1;
        parameterOffset = 0;
    }
    
    // member variables
    
        number p_05_target;
        number p_06_target;
        number notein_01_channel;
        number mtof_tilde_01_midivalue;
        list mtof_tilde_01_scale;
        list mtof_tilde_01_map;
        number mtof_tilde_01_base;
        number biquad_tilde_01_x;
        number biquad_tilde_01_a0;
        number biquad_tilde_01_a1;
        number biquad_tilde_01_a2;
        number biquad_tilde_01_b1;
        number biquad_tilde_01_b2;
        number p_07_target;
        number slide_tilde_03_x;
        number slide_tilde_03_up;
        number slide_tilde_03_down;
        number param_13_value;
        number slide_tilde_04_x;
        number slide_tilde_04_up;
        number slide_tilde_04_down;
        number slide_tilde_05_x;
        number slide_tilde_05_up;
        number slide_tilde_05_down;
        number filtercoeff_tilde_01_frequency;
        number filtercoeff_tilde_01_gain;
        number filtercoeff_tilde_01_q;
        Int filtercoeff_tilde_01_type;
        number param_14_value;
        number slide_tilde_06_x;
        number slide_tilde_06_up;
        number slide_tilde_06_down;
        number slide_tilde_07_x;
        number slide_tilde_07_up;
        number slide_tilde_07_down;
        number param_15_value;
        number ip_08_value;
        number ip_08_impulse;
        number expr_17_in1;
        number expr_17_in2;
        number expr_17_out1;
        number slide_tilde_08_x;
        number slide_tilde_08_up;
        number slide_tilde_08_down;
        number slide_tilde_09_x;
        number slide_tilde_09_up;
        number slide_tilde_09_down;
        number dspexpr_18_in1;
        number dspexpr_18_in2;
        number dspexpr_18_in3;
        number dspexpr_19_in1;
        number dspexpr_19_in2;
        number slide_tilde_10_x;
        number slide_tilde_10_up;
        number slide_tilde_10_down;
        number param_16_value;
        number slide_tilde_11_x;
        number slide_tilde_11_up;
        number slide_tilde_11_down;
        number dspexpr_20_in1;
        number dspexpr_20_in2;
        number slide_tilde_12_x;
        number slide_tilde_12_up;
        number slide_tilde_12_down;
        number p_08_target;
        number slide_tilde_13_x;
        number slide_tilde_13_up;
        number slide_tilde_13_down;
        number param_17_value;
        number param_18_value;
        number numberobj_01_value;
        number numberobj_02_value;
        number numberobj_03_value;
        number numberobj_04_value;
        number ctlin_13_input;
        number ctlin_13_controller;
        number ctlin_13_channel;
        number expr_14_in1;
        number expr_14_in2;
        number expr_14_out1;
        number ctlin_14_input;
        number ctlin_14_controller;
        number ctlin_14_channel;
        number expr_15_in1;
        number expr_15_in2;
        number expr_15_out1;
        number ctlin_15_input;
        number ctlin_15_controller;
        number ctlin_15_channel;
        number expr_16_in1;
        number expr_16_in2;
        number expr_16_out1;
        number ctlin_16_input;
        number ctlin_16_controller;
        number ctlin_16_channel;
        number expr_18_in1;
        number expr_18_in2;
        number expr_18_out1;
        MillisecondTime _currentTime;
        UInt64 audioProcessSampleCount;
        Index sampleOffsetIntoNextAudioBuffer;
        signal zeroBuffer;
        signal dummyBuffer;
        SampleValue * signals[12];
        bool didAllocateSignals;
        Index vs;
        Index maxvs;
        number sr;
        number invsr;
        Int notein_01_status;
        Int notein_01_byte1;
        Int notein_01_inchan;
        number mtof_tilde_01_innerMtoF_lastInValue;
        number mtof_tilde_01_innerMtoF_lastOutValue;
        number mtof_tilde_01_innerMtoF_lastTuning;
        SampleBufferRef mtof_tilde_01_innerMtoF_buffer;
        const Index mtof_tilde_01_innerScala_KBM_MAP_OFFSET = 7;
        bool mtof_tilde_01_innerScala_lastValid;
        number mtof_tilde_01_innerScala_lastNote;
        number mtof_tilde_01_innerScala_lastFreq;
        Int mtof_tilde_01_innerScala_sclEntryCount;
        number mtof_tilde_01_innerScala_sclOctaveMul;
        list mtof_tilde_01_innerScala_sclExpMul;
        list mtof_tilde_01_innerScala_kbmValid;
        Int mtof_tilde_01_innerScala_kbmMid;
        Int mtof_tilde_01_innerScala_kbmRefNum;
        number mtof_tilde_01_innerScala_kbmRefFreq;
        Int mtof_tilde_01_innerScala_kbmSize;
        Int mtof_tilde_01_innerScala_kbmMin;
        Int mtof_tilde_01_innerScala_kbmMax;
        Int mtof_tilde_01_innerScala_kbmOctaveDegree;
        Index mtof_tilde_01_innerScala_kbmMapSize;
        number mtof_tilde_01_innerScala_refFreq;
        number biquad_tilde_01_x1;
        number biquad_tilde_01_x2;
        number biquad_tilde_01_y1;
        number biquad_tilde_01_y2;
        bool biquad_tilde_01_setupDone;
        number slide_tilde_03_prev;
        number param_13_lastValue;
        number slide_tilde_04_prev;
        number slide_tilde_05_prev;
        number filtercoeff_tilde_01_la0;
        number filtercoeff_tilde_01_la1;
        number filtercoeff_tilde_01_la2;
        number filtercoeff_tilde_01_lb1;
        number filtercoeff_tilde_01_lb2;
        number filtercoeff_tilde_01_c_frequency;
        number filtercoeff_tilde_01_c_gain;
        number filtercoeff_tilde_01_c_q;
        Int filtercoeff_tilde_01_c_type;
        Int filtercoeff_tilde_01_resamp_counter;
        Int filtercoeff_tilde_01_resamp_value;
        bool filtercoeff_tilde_01_setupDone;
        number param_14_lastValue;
        number slide_tilde_06_prev;
        number slide_tilde_07_prev;
        number param_15_lastValue;
        SampleIndex ip_08_lastIndex;
        number ip_08_lastValue;
        SampleIndex ip_08_resetCount;
        signal ip_08_sigbuf;
        bool ip_08_setupDone;
        number slide_tilde_08_prev;
        number slide_tilde_09_prev;
        number slide_tilde_10_prev;
        number param_16_lastValue;
        number slide_tilde_11_prev;
        number slide_tilde_12_prev;
        number slide_tilde_13_prev;
        number param_17_lastValue;
        number param_18_lastValue;
        Int numberobj_01_currentFormat;
        number numberobj_01_lastValue;
        Int numberobj_02_currentFormat;
        number numberobj_02_lastValue;
        Int numberobj_03_currentFormat;
        number numberobj_03_lastValue;
        Int numberobj_04_currentFormat;
        number numberobj_04_lastValue;
        Int ctlin_13_status;
        Int ctlin_13_byte1;
        Int ctlin_13_inchan;
        Int ctlin_14_status;
        Int ctlin_14_byte1;
        Int ctlin_14_inchan;
        Int ctlin_15_status;
        Int ctlin_15_byte1;
        Int ctlin_15_inchan;
        Int ctlin_16_status;
        Int ctlin_16_byte1;
        Int ctlin_16_inchan;
        number stackprotect_count;
        Index _voiceIndex;
        Int _noteNumber;
        Index isMuted;
        ParameterIndex parameterOffset;
        RNBOSubpatcher_949 p_05;
        RNBOSubpatcher_950 p_06;
        RNBOSubpatcher_951 p_07;
        RNBOSubpatcher_952 p_08;
        bool _isInitialized = false;
};

		
void advanceTime(EXTERNALENGINE*) {}
void advanceTime(INTERNALENGINE*) {
	_internalEngine.advanceTime(sampstoms(this->vs));
}

void processInternalEvents(MillisecondTime time) {
	_internalEngine.processEventsUntil(time);
}

void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
	if (time == TimeNow) time = getPatcherTime();
	processInternalEvents(inProcess ? time + sampsToMs(this->vs) : time);
	updateTime(time, (EXTERNALENGINE*)nullptr);
}

rnbomatic* operator->() {
    return this;
}
const rnbomatic* operator->() const {
    return this;
}
rnbomatic* getTopLevelPatcher() {
    return this;
}

void cancelClockEvents()
{
}

template<typename LISTTYPE = list> void listquicksort(LISTTYPE& arr, LISTTYPE& sortindices, Int l, Int h, bool ascending) {
    if (l < h) {
        Int p = (Int)(this->listpartition(arr, sortindices, l, h, ascending));
        this->listquicksort(arr, sortindices, l, p - 1, ascending);
        this->listquicksort(arr, sortindices, p + 1, h, ascending);
    }
}

template<typename LISTTYPE = list> Int listpartition(LISTTYPE& arr, LISTTYPE& sortindices, Int l, Int h, bool ascending) {
    number x = arr[(Index)h];
    Int i = (Int)(l - 1);

    for (Int j = (Int)(l); j <= h - 1; j++) {
        bool asc = (bool)((bool)(ascending) && arr[(Index)j] <= x);
        bool desc = (bool)((bool)(!(bool)(ascending)) && arr[(Index)j] >= x);

        if ((bool)(asc) || (bool)(desc)) {
            i++;
            this->listswapelements(arr, i, j);
            this->listswapelements(sortindices, i, j);
        }
    }

    i++;
    this->listswapelements(arr, i, h);
    this->listswapelements(sortindices, i, h);
    return i;
}

template<typename LISTTYPE = list> void listswapelements(LISTTYPE& arr, Int a, Int b) {
    auto tmp = arr[(Index)a];
    arr[(Index)a] = arr[(Index)b];
    arr[(Index)b] = tmp;
}

number fromnormalized(Index index, number normalizedValue) {
    return this->convertFromNormalizedParameterValue(index, normalizedValue);
}

number mstosamps(MillisecondTime ms) {
    return ms * this->sr * 0.001;
}

number maximum(number x, number y) {
    return (x < y ? y : x);
}

MillisecondTime sampstoms(number samps) {
    return samps * 1000 / this->sr;
}

void param_19_value_set(number v) {
    v = this->param_19_value_constrain(v);
    this->param_19_value = v;
    this->sendParameter(0, false);

    if (this->param_19_value != this->param_19_lastValue) {
        this->getEngine()->presetTouched();
        this->param_19_lastValue = this->param_19_value;
    }

    this->poly_harmonicity_set(v);
}

void param_20_value_set(number v) {
    v = this->param_20_value_constrain(v);
    this->param_20_value = v;
    this->sendParameter(1, false);

    if (this->param_20_value != this->param_20_lastValue) {
        this->getEngine()->presetTouched();
        this->param_20_lastValue = this->param_20_value;
    }

    this->poly_filter_set(v);
}

void param_21_value_set(number v) {
    v = this->param_21_value_constrain(v);
    this->param_21_value = v;
    this->sendParameter(2, false);

    if (this->param_21_value != this->param_21_lastValue) {
        this->getEngine()->presetTouched();
        this->param_21_lastValue = this->param_21_value;
    }

    this->poly_q_set(v);
}

void param_22_value_set(number v) {
    v = this->param_22_value_constrain(v);
    this->param_22_value = v;
    this->sendParameter(3, false);

    if (this->param_22_value != this->param_22_lastValue) {
        this->getEngine()->presetTouched();
        this->param_22_lastValue = this->param_22_value;
    }

    this->poly_mod_index_set(v);
}

void param_23_value_set(number v) {
    v = this->param_23_value_constrain(v);
    this->param_23_value = v;
    this->sendParameter(4, false);

    if (this->param_23_value != this->param_23_lastValue) {
        this->getEngine()->presetTouched();
        this->param_23_lastValue = this->param_23_value;
    }

    this->poly_mod_index_lfo_depth_set(v);
}

void param_24_value_set(number v) {
    v = this->param_24_value_constrain(v);
    this->param_24_value = v;
    this->sendParameter(5, false);

    if (this->param_24_value != this->param_24_lastValue) {
        this->getEngine()->presetTouched();
        this->param_24_lastValue = this->param_24_value;
    }

    this->poly_modi_index_lfo_freq_set(v);
}

MillisecondTime getPatcherTime() const {
    return this->_currentTime;
}

void deallocateSignals() {
    Index i;
    this->globaltransport_tempo = freeSignal(this->globaltransport_tempo);
    this->globaltransport_state = freeSignal(this->globaltransport_state);
    this->zeroBuffer = freeSignal(this->zeroBuffer);
    this->dummyBuffer = freeSignal(this->dummyBuffer);
}

Index getMaxBlockSize() const {
    return this->maxvs;
}

number getSampleRate() const {
    return this->sr;
}

bool hasFixedVectorSize() const {
    return false;
}

void setProbingTarget(MessageTag ) {}

void fillRNBODefaultSinus(DataRef& ref) {
    SampleBuffer buffer(ref);
    number bufsize = buffer->getSize();

    for (Index i = 0; i < bufsize; i++) {
        buffer[i] = rnbo_cos(i * 3.14159265358979323846 * 2. / bufsize);
    }
}

void fillRNBODefaultMtofLookupTable256(DataRef& ref) {
    SampleBuffer buffer(ref);
    number bufsize = buffer->getSize();

    for (Index i = 0; i < bufsize; i++) {
        number midivalue = -256. + (number)512. / (bufsize - 1) * i;
        buffer[i] = rnbo_exp(.057762265 * (midivalue - 69.0));
    }
}

void fillDataRef(DataRefIndex index, DataRef& ref) {
    switch (index) {
    case 0:
        {
        this->fillRNBODefaultSinus(ref);
        break;
        }
    case 1:
        {
        this->fillRNBODefaultMtofLookupTable256(ref);
        break;
        }
    }
}

void allocateDataRefs() {
    for (Index i = 0; i < 4; i++) {
        this->poly[i]->allocateDataRefs();
    }

    if (this->RNBODefaultSinus->hasRequestedSize()) {
        if (this->RNBODefaultSinus->wantsFill())
            this->fillRNBODefaultSinus(this->RNBODefaultSinus);

        this->getEngine()->sendDataRefUpdated(0);
    }

    if (this->RNBODefaultMtofLookupTable256->hasRequestedSize()) {
        if (this->RNBODefaultMtofLookupTable256->wantsFill())
            this->fillRNBODefaultMtofLookupTable256(this->RNBODefaultMtofLookupTable256);

        this->getEngine()->sendDataRefUpdated(1);
    }
}

void initializeObjects() {
    this->midinotecontroller_01_init();

    for (Index i = 0; i < 4; i++) {
        this->poly[i]->initializeObjects();
    }
}

Index getIsMuted()  {
    return this->isMuted;
}

void setIsMuted(Index v)  {
    this->isMuted = v;
}

void onSampleRateChanged(double ) {}

void extractState(PatcherStateInterface& ) {}

void applyState() {
    for (Index i = 0; i < 4; i++) {

        this->poly[(Index)i]->setEngineAndPatcher(this->getEngine(), this);
        this->poly[(Index)i]->initialize();
        this->poly[(Index)i]->setParameterOffset(this->getParameterOffset(this->poly[0]));
        this->poly[(Index)i]->setVoiceIndex(i + 1);
    }
}

ParameterIndex getParameterOffset(BaseInterface& subpatcher) const {
    if (addressOf(subpatcher) == addressOf(this->poly[0]))
        return 6;

    return 0;
}

void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}

void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}

void processOutletEvent(
    EngineLink* sender,
    OutletIndex index,
    ParameterValue value,
    MillisecondTime time
) {
    this->updateTime(time, (ENGINE*)nullptr);
    this->processOutletAtCurrentTime(sender, index, value);
}

void sendOutlet(OutletIndex index, ParameterValue value) {
    this->getEngine()->sendOutlet(this, index, value);
}

void startup() {
    this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);

    for (Index i = 0; i < 4; i++) {
        this->poly[i]->startup();
    }

    {
        this->scheduleParamInit(0, 0);
    }

    {
        this->scheduleParamInit(1, 0);
    }

    {
        this->scheduleParamInit(2, 0);
    }

    {
        this->scheduleParamInit(3, 0);
    }

    {
        this->scheduleParamInit(4, 0);
    }

    {
        this->scheduleParamInit(5, 0);
    }

    this->processParamInitEvents();
}

number param_19_value_constrain(number v) const {
    v = (v > 10 ? 10 : (v < 1 ? 1 : v));
    return v;
}

void poly_harmonicity_set(number v) {
    for (number i = 0; i < 4; i++) {
        if (i + 1 == this->poly_target || 0 == this->poly_target) {
            this->poly[(Index)i]->setParameterValue(0, v, this->_currentTime);
        }
    }
}

number param_20_value_constrain(number v) const {
    v = (v > 20480 ? 20480 : (v < 20 ? 20 : v));
    return v;
}

void poly_filter_set(number v) {
    for (number i = 0; i < 4; i++) {
        if (i + 1 == this->poly_target || 0 == this->poly_target) {
            this->poly[(Index)i]->setParameterValue(1, v, this->_currentTime);
        }
    }
}

number param_21_value_constrain(number v) const {
    v = (v > 4 ? 4 : (v < 0.1 ? 0.1 : v));
    return v;
}

void poly_q_set(number v) {
    for (number i = 0; i < 4; i++) {
        if (i + 1 == this->poly_target || 0 == this->poly_target) {
            this->poly[(Index)i]->setParameterValue(2, v, this->_currentTime);
        }
    }
}

number param_22_value_constrain(number v) const {
    v = (v > 10 ? 10 : (v < 0.1 ? 0.1 : v));
    return v;
}

void poly_mod_index_set(number v) {
    for (number i = 0; i < 4; i++) {
        if (i + 1 == this->poly_target || 0 == this->poly_target) {
            this->poly[(Index)i]->setParameterValue(3, v, this->_currentTime);
        }
    }
}

number param_23_value_constrain(number v) const {
    v = (v > 10 ? 10 : (v < 0 ? 0 : v));
    return v;
}

void poly_mod_index_lfo_depth_set(number v) {
    for (number i = 0; i < 4; i++) {
        if (i + 1 == this->poly_target || 0 == this->poly_target) {
            this->poly[(Index)i]->setParameterValue(4, v, this->_currentTime);
        }
    }
}

number param_24_value_constrain(number v) const {
    v = (v > 40 ? 40 : (v < 0 ? 0 : v));
    return v;
}

void poly_modi_index_lfo_freq_set(number v) {
    for (number i = 0; i < 4; i++) {
        if (i + 1 == this->poly_target || 0 == this->poly_target) {
            this->poly[(Index)i]->setParameterValue(5, v, this->_currentTime);
        }
    }
}

void midinotecontroller_01_currenttarget_set(number v) {
    this->midinotecontroller_01_currenttarget = v;
}

void poly_target_set(number v) {
    this->poly_target = v;
    this->midinotecontroller_01_currenttarget_set(v);
}

void midinotecontroller_01_target_set(number v) {
    this->poly_target_set(v);
}

void poly_midiininternal_set(number v) {
    Index sendlen = 0;
    this->poly_currentStatus = parseMidi(this->poly_currentStatus, (Int)(v), this->poly_mididata[0]);

    switch ((int)this->poly_currentStatus) {
    case MIDI_StatusByteReceived:
        {
        this->poly_mididata[0] = (uint8_t)(v);
        this->poly_mididata[1] = 0;
        break;
        }
    case MIDI_SecondByteReceived:
    case MIDI_ProgramChange:
    case MIDI_ChannelPressure:
        {
        this->poly_mididata[1] = (uint8_t)(v);

        if (this->poly_currentStatus == MIDI_ProgramChange || this->poly_currentStatus == MIDI_ChannelPressure) {
            sendlen = 2;
        }

        break;
        }
    case MIDI_NoteOff:
    case MIDI_NoteOn:
    case MIDI_Aftertouch:
    case MIDI_CC:
    case MIDI_PitchBend:
    default:
        {
        this->poly_mididata[2] = (uint8_t)(v);
        sendlen = 3;
        break;
        }
    }

    if (sendlen > 0) {
        number i;

        if (this->poly_target > 0 && this->poly_target <= 4) {
            i = this->poly_target - 1;
            this->poly[(Index)i]->processMidiEvent(_currentTime, 0, this->poly_mididata, sendlen);
        } else if (this->poly_target == 0) {
            for (i = 0; i < 4; i++) {
                this->poly[(Index)i]->processMidiEvent(_currentTime, 0, this->poly_mididata, sendlen);
            }
        }
    }
}

void midinotecontroller_01_midiout_set(number v) {
    this->poly_midiininternal_set(v);
}

void poly_noteNumber_set(number v) {
    if (this->poly_target > 0) {
        this->poly[(Index)(this->poly_target - 1)]->setNoteNumber((Int)(v));
    }
}

void midinotecontroller_01_noteNumber_set(number v) {
    this->poly_noteNumber_set(v);
}

template<typename LISTTYPE> void midinotecontroller_01_voicestatus_set(const LISTTYPE& v) {
    if (v[1] == 1) {
        number currentTarget = this->midinotecontroller_01_currenttarget;
        this->midinotecontroller_01_target_set(v[0]);
        this->midinotecontroller_01_noteNumber_set(0);
        this->midinotecontroller_01_target_set(currentTarget);
    }
}

template<typename LISTTYPE> void poly_voicestatus_set(const LISTTYPE& v) {
    this->midinotecontroller_01_voicestatus_set(v);
}

void poly_activevoices_set(number ) {}

template<typename LISTTYPE> void poly_mute_set(const LISTTYPE& v) {
    Index voiceNumber = (Index)(v[0]);
    Index muteState = (Index)(v[1]);

    if (voiceNumber == 0) {
        for (Index i = 0; i < 4; i++) {
            this->poly[(Index)i]->setIsMuted(muteState);
        }
    } else {
        Index subpatcherIndex = voiceNumber - 1;

        if (subpatcherIndex >= 0 && subpatcherIndex < 4) {
            this->poly[(Index)subpatcherIndex]->setIsMuted(muteState);
        }
    }

    list tmp = {v[0], v[1]};
    this->poly_voicestatus_set(tmp);
    this->poly_activevoices_set(this->poly_calcActiveVoices());
}

template<typename LISTTYPE> void midinotecontroller_01_mute_set(const LISTTYPE& v) {
    this->poly_mute_set(v);
}

void midinotecontroller_01_midiin_set(number v) {
    this->midinotecontroller_01_midiin = v;
    Int val = (Int)(v);

    this->midinotecontroller_01_currentStatus = parseMidi(
        this->midinotecontroller_01_currentStatus,
        (Int)(v),
        this->midinotecontroller_01_status
    );

    switch ((int)this->midinotecontroller_01_currentStatus) {
    case MIDI_StatusByteReceived:
        {
        {
            this->midinotecontroller_01_status = val;
            this->midinotecontroller_01_byte1 = -1;
            break;
        }
        }
    case MIDI_SecondByteReceived:
        {
        this->midinotecontroller_01_byte1 = val;
        break;
        }
    case MIDI_NoteOn:
        {
        {
            bool sendnoteoff = true;
            Int target = 1;
            MillisecondTime oldest = (MillisecondTime)(this->midinotecontroller_01_voice_lastontime[0]);
            number target_state = this->midinotecontroller_01_voice_state[0];

            for (Int i = 0; i < 4; i++) {
                number candidate_state = this->midinotecontroller_01_voice_state[(Index)i];

                if (this->midinotecontroller_01_voice_notenumber[(Index)i] == this->midinotecontroller_01_byte1 && candidate_state == MIDI_NoteState_On) {
                    sendnoteoff = false;
                    target = i + 1;
                    break;
                }

                if (i > 0) {
                    if (candidate_state != MIDI_NoteState_On || target_state == MIDI_NoteState_On) {
                        MillisecondTime candidate_ontime = (MillisecondTime)(this->midinotecontroller_01_voice_lastontime[(Index)i]);

                        if (candidate_ontime < oldest || (target_state == MIDI_NoteState_On && candidate_state != MIDI_NoteState_On)) {
                            target = i + 1;
                            oldest = candidate_ontime;
                            target_state = candidate_state;
                        }
                    }
                }
            }

            if ((bool)(sendnoteoff))
                this->midinotecontroller_01_sendnoteoff(target);

            Int i = (Int)(target - 1);
            this->midinotecontroller_01_voice_state[(Index)i] = MIDI_NoteState_On;
            this->midinotecontroller_01_voice_lastontime[(Index)i] = this->_currentTime;
            this->midinotecontroller_01_voice_notenumber[(Index)i] = this->midinotecontroller_01_byte1;
            this->midinotecontroller_01_voice_channel[(Index)i] = (BinOpInt)((BinOpInt)this->midinotecontroller_01_status & (BinOpInt)0x0F);

            for (Index j = 0; j < 128; j++) {
                if (this->midinotecontroller_01_notesdown[(Index)j] == 0) {
                    this->midinotecontroller_01_notesdown[(Index)j] = this->midinotecontroller_01_voice_notenumber[(Index)i];
                    break;
                }
            }

            this->midinotecontroller_01_note_lastvelocity[(Index)this->midinotecontroller_01_voice_notenumber[(Index)i]] = val;
            this->midinotecontroller_01_sendpitchbend(i);
            this->midinotecontroller_01_sendpressure(i);
            this->midinotecontroller_01_sendtimbre(i);
            this->midinotecontroller_01_muteval[0] = target;
            this->midinotecontroller_01_muteval[1] = 0;
            this->midinotecontroller_01_mute_set(this->midinotecontroller_01_muteval);
            number currentTarget = this->midinotecontroller_01_currenttarget;
            this->midinotecontroller_01_target_set(target);
            this->midinotecontroller_01_noteNumber_set(this->midinotecontroller_01_voice_notenumber[(Index)i]);

            this->midinotecontroller_01_midiout_set(
                (BinOpInt)((BinOpInt)MIDI_NoteOnMask | (BinOpInt)this->midinotecontroller_01_voice_channel[(Index)i])
            );

            this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_voice_notenumber[(Index)i]);
            this->midinotecontroller_01_midiout_set(val);
            this->midinotecontroller_01_target_set(currentTarget);
            break;
        }
        }
    case MIDI_NoteOff:
        {
        {
            Int target = 0;
            number notenumber = this->midinotecontroller_01_byte1;
            number channel = (BinOpInt)((BinOpInt)this->midinotecontroller_01_status & (BinOpInt)0x0F);

            for (Int i = 0; i < 4; i++) {
                if (this->midinotecontroller_01_voice_notenumber[(Index)i] == notenumber && this->midinotecontroller_01_voice_channel[(Index)i] == channel && this->midinotecontroller_01_voice_state[(Index)i] == MIDI_NoteState_On) {
                    target = i + 1;
                    break;
                }
            }

            if (target > 0) {
                Int i = (Int)(target - 1);
                Index j = (Index)(channel);
                bool ignoresustainchannel = true;

                if ((bool)(this->midinotecontroller_01_channel_sustain[((bool)(ignoresustainchannel) ? 0 : j)])) {
                    this->midinotecontroller_01_voice_state[(Index)i] = MIDI_NoteState_Sustained;
                } else {
                    number currentTarget = this->midinotecontroller_01_currenttarget;
                    this->midinotecontroller_01_target_set(target);
                    this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_status);
                    this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_byte1);
                    this->midinotecontroller_01_midiout_set(v);
                    this->midinotecontroller_01_target_set(currentTarget);

                    if (this->midinotecontroller_01_currentStatus == MIDI_NoteOff) {
                        this->midinotecontroller_01_voice_state[(Index)i] = MIDI_NoteState_Off;
                    }
                }
            } else
                {}

            bool found = false;

            for (Index i = 0; i < 128; i++) {
                if (this->midinotecontroller_01_notesdown[(Index)i] == 0) {
                    break;
                } else if (this->midinotecontroller_01_notesdown[(Index)i] == notenumber) {
                    found = true;
                }

                if ((bool)(found)) {
                    this->midinotecontroller_01_notesdown[(Index)i] = this->midinotecontroller_01_notesdown[(Index)(i + 1)];
                }
            }

            break;
        }
        }
    case MIDI_Aftertouch:
        {
        {
            number currentTarget = this->midinotecontroller_01_currenttarget;
            this->midinotecontroller_01_target_set(0);
            this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_status);
            this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_byte1);
            this->midinotecontroller_01_midiout_set(v);
            this->midinotecontroller_01_target_set(currentTarget);
            break;
        }
        }
    case MIDI_CC:
        {
        {
            bool sendToAllVoices = true;

            switch ((int)this->midinotecontroller_01_byte1) {
            case MIDI_CC_Sustain:
                {
                {
                    bool pedaldown = (bool)((val >= 64 ? true : false));
                    number channel = (BinOpInt)((BinOpInt)this->midinotecontroller_01_status & (BinOpInt)0x0F);
                    Index j = (Index)(channel);
                    bool ignoresustainchannel = true;
                    this->midinotecontroller_01_channel_sustain[((bool)(ignoresustainchannel) ? 0 : j)] = pedaldown;

                    if ((bool)(!(bool)(pedaldown))) {
                        for (Index i = 0; i < 4; i++) {
                            if (((bool)(ignoresustainchannel) || this->midinotecontroller_01_voice_channel[(Index)i] == channel) && this->midinotecontroller_01_voice_state[(Index)i] == MIDI_NoteState_Sustained) {
                                number currentTarget = this->midinotecontroller_01_currenttarget;
                                this->midinotecontroller_01_target_set(i + 1);
                                this->midinotecontroller_01_midiout_set((BinOpInt)((BinOpInt)MIDI_NoteOffMask | (BinOpInt)j));
                                this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_voice_notenumber[(Index)i]);
                                this->midinotecontroller_01_midiout_set(64);
                                this->midinotecontroller_01_target_set(currentTarget);
                                this->midinotecontroller_01_voice_state[(Index)i] = MIDI_NoteState_Off;
                            }
                        }
                    }

                    break;
                }
                }
            case MIDI_CC_TimbreMSB:
                {
                {
                    number channel = (BinOpInt)((BinOpInt)this->midinotecontroller_01_status & (BinOpInt)0x0F);
                    Int k = (Int)(v);
                    number timbre = (BinOpInt)(((BinOpInt)((BinOpInt)k & (BinOpInt)0x7F)) << imod_nocast((UBinOpInt)7, 32));
                    this->midinotecontroller_01_channel_timbre[(Index)((BinOpInt)this->midinotecontroller_01_status & (BinOpInt)0x0F)] = timbre;

                    for (Int i = 0; i < 4; i++) {
                        if (this->midinotecontroller_01_voice_channel[(Index)i] == channel && this->midinotecontroller_01_voice_state[(Index)i] != MIDI_NoteState_Off) {
                            this->midinotecontroller_01_sendtimbre(i);
                        }
                    }

                    sendToAllVoices = false;
                    break;
                }
                }
            case MIDI_CC_TimbreLSB:
                {
                {
                    break;
                }
                }
            case MIDI_CC_AllNotesOff:
                {
                {
                    this->midinotecontroller_01_sendallnotesoff();
                    break;
                }
                }
            }

            if ((bool)(sendToAllVoices)) {
                number currentTarget = this->midinotecontroller_01_currenttarget;
                this->midinotecontroller_01_target_set(0);
                this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_status);
                this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_byte1);
                this->midinotecontroller_01_midiout_set(v);
                this->midinotecontroller_01_target_set(currentTarget);
            }

            break;
        }
        }
    case MIDI_ProgramChange:
        {
        {
            number currentTarget = this->midinotecontroller_01_currenttarget;
            this->midinotecontroller_01_target_set(0);
            this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_status);
            this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_byte1);
            this->midinotecontroller_01_target_set(currentTarget);
            break;
        }
        }
    case MIDI_ChannelPressure:
        {
        {
            number channel = (BinOpInt)((BinOpInt)this->midinotecontroller_01_status & (BinOpInt)0x0F);

            for (Int i = 0; i < 4; i++) {
                if (this->midinotecontroller_01_voice_state[(Index)i] != MIDI_NoteState_Off && this->midinotecontroller_01_voice_channel[(Index)i] == channel) {
                    Int k = (Int)(channel);
                    this->midinotecontroller_01_channel_pressure[(Index)k] = v;
                    this->midinotecontroller_01_sendpressure(i);
                }
            }

            break;
        }
        }
    case MIDI_PitchBend:
        {
        {
            number bendamount = (BinOpInt)((BinOpInt)this->midinotecontroller_01_byte1 | (BinOpInt)((BinOpInt)val << imod_nocast((UBinOpInt)7, 32)));
            Int channel = (Int)((BinOpInt)((BinOpInt)this->midinotecontroller_01_status & (BinOpInt)0x0F));
            this->midinotecontroller_01_channel_pitchbend[(Index)channel] = bendamount;

            for (Int i = 0; i < 4; i++) {
                if (this->midinotecontroller_01_voice_state[(Index)i] != MIDI_NoteState_Off && this->midinotecontroller_01_voice_channel[(Index)i] == channel) {
                    this->midinotecontroller_01_sendpitchbend(i);
                }
            }

            break;
        }
        }
    }
}

void poly_midiin_set(number v) {
    this->poly_midiin = v;
    this->midinotecontroller_01_midiin_set(v);
}

void midiin_midiout_set(number v) {
    this->poly_midiin_set(v);
}

void midiin_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
    RNBO_UNUSED(port);
    RNBO_UNUSED(channel);
    RNBO_UNUSED(status);
    Index i;

    for (i = 0; i < length; i++) {
        this->midiin_midiout_set(data[i]);
    }
}

void ctlin_17_outchannel_set(number ) {}

void ctlin_17_outcontroller_set(number ) {}

void fromnormalized_17_output_set(number v) {
    this->param_19_value_set(v);
}

void fromnormalized_17_input_set(number v) {
    this->fromnormalized_17_output_set(this->fromnormalized(0, v));
}

void expr_19_out1_set(number v) {
    this->expr_19_out1 = v;
    this->fromnormalized_17_input_set(this->expr_19_out1);
}

void expr_19_in1_set(number in1) {
    this->expr_19_in1 = in1;
    this->expr_19_out1_set(this->expr_19_in1 * this->expr_19_in2);//#map:expr_19:1
}

void ctlin_17_value_set(number v) {
    this->expr_19_in1_set(v);
}

void ctlin_17_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
    RNBO_UNUSED(length);
    RNBO_UNUSED(port);

    if (status == 0xB0 && (channel == this->ctlin_17_channel || this->ctlin_17_channel == -1) && (data[1] == this->ctlin_17_controller || this->ctlin_17_controller == -1)) {
        this->ctlin_17_outchannel_set(channel);
        this->ctlin_17_outcontroller_set(data[1]);
        this->ctlin_17_value_set(data[2]);
        this->ctlin_17_status = 0;
    }
}

void ctlin_18_outchannel_set(number ) {}

void ctlin_18_outcontroller_set(number ) {}

void fromnormalized_18_output_set(number v) {
    this->param_20_value_set(v);
}

void fromnormalized_18_input_set(number v) {
    this->fromnormalized_18_output_set(this->fromnormalized(1, v));
}

void expr_20_out1_set(number v) {
    this->expr_20_out1 = v;
    this->fromnormalized_18_input_set(this->expr_20_out1);
}

void expr_20_in1_set(number in1) {
    this->expr_20_in1 = in1;
    this->expr_20_out1_set(this->expr_20_in1 * this->expr_20_in2);//#map:expr_20:1
}

void ctlin_18_value_set(number v) {
    this->expr_20_in1_set(v);
}

void ctlin_18_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
    RNBO_UNUSED(length);
    RNBO_UNUSED(port);

    if (status == 0xB0 && (channel == this->ctlin_18_channel || this->ctlin_18_channel == -1) && (data[1] == this->ctlin_18_controller || this->ctlin_18_controller == -1)) {
        this->ctlin_18_outchannel_set(channel);
        this->ctlin_18_outcontroller_set(data[1]);
        this->ctlin_18_value_set(data[2]);
        this->ctlin_18_status = 0;
    }
}

void ctlin_19_outchannel_set(number ) {}

void ctlin_19_outcontroller_set(number ) {}

void fromnormalized_19_output_set(number v) {
    this->param_21_value_set(v);
}

void fromnormalized_19_input_set(number v) {
    this->fromnormalized_19_output_set(this->fromnormalized(2, v));
}

void expr_21_out1_set(number v) {
    this->expr_21_out1 = v;
    this->fromnormalized_19_input_set(this->expr_21_out1);
}

void expr_21_in1_set(number in1) {
    this->expr_21_in1 = in1;
    this->expr_21_out1_set(this->expr_21_in1 * this->expr_21_in2);//#map:expr_21:1
}

void ctlin_19_value_set(number v) {
    this->expr_21_in1_set(v);
}

void ctlin_19_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
    RNBO_UNUSED(length);
    RNBO_UNUSED(port);

    if (status == 0xB0 && (channel == this->ctlin_19_channel || this->ctlin_19_channel == -1) && (data[1] == this->ctlin_19_controller || this->ctlin_19_controller == -1)) {
        this->ctlin_19_outchannel_set(channel);
        this->ctlin_19_outcontroller_set(data[1]);
        this->ctlin_19_value_set(data[2]);
        this->ctlin_19_status = 0;
    }
}

void ctlin_20_outchannel_set(number ) {}

void ctlin_20_outcontroller_set(number ) {}

void fromnormalized_20_output_set(number v) {
    this->param_22_value_set(v);
}

void fromnormalized_20_input_set(number v) {
    this->fromnormalized_20_output_set(this->fromnormalized(3, v));
}

void expr_22_out1_set(number v) {
    this->expr_22_out1 = v;
    this->fromnormalized_20_input_set(this->expr_22_out1);
}

void expr_22_in1_set(number in1) {
    this->expr_22_in1 = in1;
    this->expr_22_out1_set(this->expr_22_in1 * this->expr_22_in2);//#map:expr_22:1
}

void ctlin_20_value_set(number v) {
    this->expr_22_in1_set(v);
}

void ctlin_20_midihandler(int status, int channel, int port, ConstByteArray data, Index length) {
    RNBO_UNUSED(length);
    RNBO_UNUSED(port);

    if (status == 0xB0 && (channel == this->ctlin_20_channel || this->ctlin_20_channel == -1) && (data[1] == this->ctlin_20_controller || this->ctlin_20_controller == -1)) {
        this->ctlin_20_outchannel_set(channel);
        this->ctlin_20_outcontroller_set(data[1]);
        this->ctlin_20_value_set(data[2]);
        this->ctlin_20_status = 0;
    }
}

void poly_perform(SampleValue * out1, SampleValue * out2, SampleValue * out3, Index n) {
    SampleArray<3> outs = {out1, out2, out3};

    for (number chan = 0; chan < 3; chan++)
        zeroSignal(outs[(Index)chan], n);

    for (Index i = 0; i < 4; i++)
        this->poly[(Index)i]->process(nullptr, 0, outs, 3, n);
}

void stackprotect_perform(Index n) {
    RNBO_UNUSED(n);
    auto __stackprotect_count = this->stackprotect_count;
    __stackprotect_count = 0;
    this->stackprotect_count = __stackprotect_count;
}

void param_19_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_19_value;
}

void param_19_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_19_value_set(preset["value"]);
}

void param_20_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_20_value;
}

void param_20_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_20_value_set(preset["value"]);
}

void param_21_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_21_value;
}

void param_21_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_21_value_set(preset["value"]);
}

void param_22_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_22_value;
}

void param_22_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_22_value_set(preset["value"]);
}

void param_23_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_23_value;
}

void param_23_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_23_value_set(preset["value"]);
}

void param_24_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_24_value;
}

void param_24_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_24_value_set(preset["value"]);
}

number poly_calcActiveVoices() {
    {
        number activeVoices = 0;

        for (Index i = 0; i < 4; i++) {
            if ((bool)(!(bool)(this->poly[(Index)i]->getIsMuted())))
                activeVoices++;
        }

        return activeVoices;
    }
}

void midinotecontroller_01_init() {
    for (Index i = 0; i < 16; i++) {
        this->midinotecontroller_01_channel_pitchbend[(Index)i] = 0x2000;
    }

    for (Index i = 0; i < 4; i++) {
        this->midinotecontroller_01_voice_lastontime[(Index)i] = -1;
    }
}

void midinotecontroller_01_sendnoteoff(Int target) {
    Int i = (Int)(target - 1);

    if (this->midinotecontroller_01_voice_state[(Index)i] != MIDI_NoteState_Off) {
        number currentTarget = this->midinotecontroller_01_currenttarget;
        this->midinotecontroller_01_target_set(target);

        this->midinotecontroller_01_midiout_set(
            (BinOpInt)((BinOpInt)MIDI_NoteOffMask | (BinOpInt)this->midinotecontroller_01_voice_channel[(Index)i])
        );

        this->midinotecontroller_01_midiout_set(this->midinotecontroller_01_voice_notenumber[(Index)i]);
        this->midinotecontroller_01_midiout_set(64);
        this->midinotecontroller_01_voice_state[(Index)i] = MIDI_NoteState_Off;
        this->midinotecontroller_01_target_set(currentTarget);
    }
}

void midinotecontroller_01_sendpitchbend(Int v) {
    if (v >= 0 && v < 4) {
        number currentTarget = this->midinotecontroller_01_currenttarget;
        this->midinotecontroller_01_target_set(v + 1);
        Int totalbendamount = (Int)(this->midinotecontroller_01_channel_pitchbend[(Index)this->midinotecontroller_01_voice_channel[(Index)v]]);

        this->midinotecontroller_01_midiout_set(
            (BinOpInt)((BinOpInt)MIDI_PitchBendMask | (BinOpInt)this->midinotecontroller_01_voice_channel[(Index)v])
        );

        this->midinotecontroller_01_midiout_set((BinOpInt)((BinOpInt)totalbendamount & (BinOpInt)0x7F));

        this->midinotecontroller_01_midiout_set(
            (BinOpInt)((BinOpInt)((BinOpInt)totalbendamount >> imod_nocast((UBinOpInt)imod_nocast((UBinOpInt)7, 32), 32)) & (BinOpInt)0x7F)
        );

        this->midinotecontroller_01_target_set(currentTarget);
    }
}

void midinotecontroller_01_sendpressure(Int v) {
    number currentTarget = this->midinotecontroller_01_currenttarget;
    this->midinotecontroller_01_target_set(v + 1);

    this->midinotecontroller_01_midiout_set(
        (BinOpInt)((BinOpInt)MIDI_ChannelPressureMask | (BinOpInt)this->midinotecontroller_01_voice_channel[(Index)v])
    );

    this->midinotecontroller_01_midiout_set(
        this->midinotecontroller_01_channel_pressure[(Index)this->midinotecontroller_01_voice_channel[(Index)v]]
    );

    this->midinotecontroller_01_target_set(currentTarget);
}

void midinotecontroller_01_sendtimbre(Int v) {
    number currentTarget = this->midinotecontroller_01_currenttarget;
    this->midinotecontroller_01_target_set(v + 1);

    this->midinotecontroller_01_midiout_set(
        (BinOpInt)((BinOpInt)MIDI_CCMask | (BinOpInt)this->midinotecontroller_01_voice_channel[(Index)v])
    );

    this->midinotecontroller_01_midiout_set(MIDI_CC_TimbreLSB);

    this->midinotecontroller_01_midiout_set(
        (BinOpInt)((BinOpInt)this->midinotecontroller_01_channel_timbre[(Index)this->midinotecontroller_01_voice_channel[(Index)v]] & (BinOpInt)0x7F)
    );

    this->midinotecontroller_01_midiout_set(
        (BinOpInt)((BinOpInt)MIDI_CCMask | (BinOpInt)this->midinotecontroller_01_voice_channel[(Index)v])
    );

    this->midinotecontroller_01_midiout_set(MIDI_CC_TimbreMSB);

    this->midinotecontroller_01_midiout_set(
        (BinOpInt)((BinOpInt)((BinOpInt)this->midinotecontroller_01_channel_timbre[(Index)this->midinotecontroller_01_voice_channel[(Index)v]] >> imod_nocast((UBinOpInt)7, 32)) & (BinOpInt)0x7F)
    );

    this->midinotecontroller_01_target_set(currentTarget);
}

void midinotecontroller_01_sendallnotesoff() {
    for (Int i = 1; i <= 4; i++) {
        this->midinotecontroller_01_sendnoteoff(i);
    }
}

number globaltransport_getSampleOffset(MillisecondTime time) {
    return this->mstosamps(this->maximum(0, time - this->getEngine()->getCurrentTime()));
}

number globaltransport_getTempoAtSample(SampleIndex sampleOffset) {
    return (sampleOffset >= 0 && sampleOffset < (SampleIndex)(this->vs) ? this->globaltransport_tempo[(Index)sampleOffset] : this->globaltransport_lastTempo);
}

number globaltransport_getStateAtSample(SampleIndex sampleOffset) {
    return (sampleOffset >= 0 && sampleOffset < (SampleIndex)(this->vs) ? this->globaltransport_state[(Index)sampleOffset] : this->globaltransport_lastState);
}

number globaltransport_getState(MillisecondTime time) {
    return this->globaltransport_getStateAtSample((SampleIndex)(this->globaltransport_getSampleOffset(time)));
}

number globaltransport_getTempo(MillisecondTime time) {
    return this->globaltransport_getTempoAtSample((SampleIndex)(this->globaltransport_getSampleOffset(time)));
}

number globaltransport_getBeatTime(MillisecondTime time) {
    number i = 2;

    while (i < this->globaltransport_beatTimeChanges->length && this->globaltransport_beatTimeChanges[(Index)(i + 1)] <= time) {
        i += 2;
    }

    i -= 2;
    number beatTimeBase = this->globaltransport_beatTimeChanges[(Index)i];

    if (this->globaltransport_getState(time) == 0)
        return beatTimeBase;

    number beatTimeBaseMsTime = this->globaltransport_beatTimeChanges[(Index)(i + 1)];
    number diff = time - beatTimeBaseMsTime;
    number diffInBeats = diff * this->globaltransport_getTempo(time) * 0.008 / (number)480;
    return beatTimeBase + diffInBeats;
}

bool globaltransport_setTempo(MillisecondTime time, number tempo, bool notify) {
    if ((bool)(notify)) {
        this->processTempoEvent(time, tempo);
        this->globaltransport_notify = true;
    } else {
        Index offset = (Index)(this->globaltransport_getSampleOffset(time));

        if (this->globaltransport_getTempoAtSample((SampleIndex)(offset)) != tempo) {
            this->globaltransport_beatTimeChanges->push(this->globaltransport_getBeatTime(time));
            this->globaltransport_beatTimeChanges->push(time);
            fillSignal(this->globaltransport_tempo, this->vs, tempo, offset);
            this->globaltransport_lastTempo = tempo;
            this->globaltransport_tempoNeedsReset = true;
            return true;
        }
    }

    return false;
}

bool globaltransport_setState(MillisecondTime time, number state, bool notify) {
    if ((bool)(notify)) {
        this->processTransportEvent(time, TransportState(state));
        this->globaltransport_notify = true;
    } else {
        SampleIndex offset = (SampleIndex)(this->globaltransport_getSampleOffset(time));

        if (this->globaltransport_getStateAtSample(offset) != state) {
            this->globaltransport_beatTimeChanges->push(this->globaltransport_getBeatTime(time));
            this->globaltransport_beatTimeChanges->push(time);
            fillSignal(this->globaltransport_state, this->vs, state, (Index)(offset));
            this->globaltransport_lastState = TransportState(state);
            this->globaltransport_stateNeedsReset = true;
            return true;
        }
    }

    return false;
}

bool globaltransport_setBeatTime(MillisecondTime time, number beattime, bool notify) {
    if ((bool)(notify)) {
        this->processBeatTimeEvent(time, beattime);
        this->globaltransport_notify = true;
        return false;
    } else {
        bool beatTimeHasChanged = false;

        if (rnbo_abs(beattime - this->globaltransport_getBeatTime(time)) > 0.001) {
            beatTimeHasChanged = true;
        }

        this->globaltransport_beatTimeChanges->push(beattime);
        this->globaltransport_beatTimeChanges->push(time);
        return beatTimeHasChanged;
    }
}

number globaltransport_getBeatTimeAtSample(SampleIndex sampleOffset) {
    auto msOffset = this->sampstoms(sampleOffset);
    return this->globaltransport_getBeatTime(this->getEngine()->getCurrentTime() + msOffset);
}

array<number, 2> globaltransport_getTimeSignature(MillisecondTime time) {
    number i = 3;

    while (i < this->globaltransport_timeSignatureChanges->length && this->globaltransport_timeSignatureChanges[(Index)(i + 2)] <= time) {
        i += 3;
    }

    i -= 3;

    return {
        this->globaltransport_timeSignatureChanges[(Index)i],
        this->globaltransport_timeSignatureChanges[(Index)(i + 1)]
    };
}

array<number, 2> globaltransport_getTimeSignatureAtSample(SampleIndex sampleOffset) {
    MillisecondTime msOffset = (MillisecondTime)(this->sampstoms(sampleOffset));
    return this->globaltransport_getTimeSignature(this->getEngine()->getCurrentTime() + msOffset);
}

void globaltransport_setBBUBase(
    MillisecondTime time,
    number numerator,
    number denominator,
    number bars,
    number beats,
    number units
) {
    number beatsInQuarterNotes = this->globaltransport_getBeatTime(time);
    bars--;
    beats--;
    number beatsIncCurrenttDenom = beatsInQuarterNotes * (denominator * 0.25);
    number beatLength = (number)4 / denominator;
    number beatLengthInUnits = beatLength * 480;

    while (units > beatLengthInUnits) {
        units -= beatLengthInUnits;
        beats++;
    }

    number targetBeatTime = bars * numerator + beats + units / beatLengthInUnits;
    this->globaltransport_bbuBase = targetBeatTime - beatsIncCurrenttDenom;
}

array<number, 3> globaltransport_getBBU(MillisecondTime time) {
    array<number, 2> currentSig = this->globaltransport_getTimeSignature(time);
    number numerator = currentSig[0];
    number denominator = currentSig[1];
    number beatsInQuarterNotes = this->globaltransport_getBeatTime(time);
    number beatsIncCurrenttDenom = beatsInQuarterNotes * (denominator * 0.25);
    number beatLength = (number)4 / denominator;
    number beatLengthInUnits = beatLength * 480;
    number targetBeatTime = beatsIncCurrenttDenom + this->globaltransport_bbuBase;
    number currentBars = 0;
    number currentBeats = 0;
    number currentUnits = 0;

    if (targetBeatTime >= 0) {
        currentBars = trunc(targetBeatTime / numerator);
        targetBeatTime -= currentBars * numerator;
        currentBeats = trunc(targetBeatTime);
        targetBeatTime -= currentBeats;
        currentUnits = targetBeatTime * beatLengthInUnits;
    } else {
        currentBars = trunc(targetBeatTime / numerator);
        targetBeatTime -= currentBars * numerator;

        if (targetBeatTime != 0) {
            currentBars -= 1;
            currentBeats = trunc(targetBeatTime);
            targetBeatTime -= currentBeats;
            currentBeats = numerator + currentBeats;
            currentUnits = targetBeatTime * beatLengthInUnits;

            if (currentUnits != 0) {
                currentUnits = beatLengthInUnits + currentUnits;
                currentBeats -= 1;
            }
        }
    }

    return {currentBars + 1, currentBeats + 1, currentUnits};
}

bool globaltransport_setTimeSignature(MillisecondTime time, number numerator, number denominator, bool notify) {
    if ((bool)(notify)) {
        this->processTimeSignatureEvent(time, (Int)(numerator), (Int)(denominator));
        this->globaltransport_notify = true;
    } else {
        array<number, 2> currentSig = this->globaltransport_getTimeSignature(time);

        if (currentSig[0] != numerator || currentSig[1] != denominator) {
            array<number, 3> bbu = this->globaltransport_getBBU(time);
            this->globaltransport_setBBUBase(time, numerator, denominator, bbu[0], bbu[1], bbu[2]);
            this->globaltransport_timeSignatureChanges->push(numerator);
            this->globaltransport_timeSignatureChanges->push(denominator);
            this->globaltransport_timeSignatureChanges->push(time);
            return true;
        }
    }

    return false;
}

array<number, 3> globaltransport_getBBUAtSample(SampleIndex sampleOffset) {
    auto msOffset = this->sampstoms(sampleOffset);
    return this->globaltransport_getBBU(this->getEngine()->getCurrentTime() + msOffset);
}

bool globaltransport_setBBU(MillisecondTime time, number bars, number beats, number units, bool notify) {
    RNBO_UNUSED(notify);
    array<number, 2> sig = this->globaltransport_getTimeSignature(time);
    number numerator = sig[0];
    number denominator = sig[1];
    this->globaltransport_setBBUBase(time, numerator, denominator, bars, beats, units);
    return true;
}

void globaltransport_advance() {
    if ((bool)(this->globaltransport_tempoNeedsReset)) {
        fillSignal(this->globaltransport_tempo, this->vs, this->globaltransport_lastTempo);
        this->globaltransport_tempoNeedsReset = false;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendTempoEvent(this->globaltransport_lastTempo);
        }
    }

    if ((bool)(this->globaltransport_stateNeedsReset)) {
        fillSignal(this->globaltransport_state, this->vs, this->globaltransport_lastState);
        this->globaltransport_stateNeedsReset = false;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendTransportEvent(TransportState(this->globaltransport_lastState));
        }
    }

    if (this->globaltransport_beatTimeChanges->length > 2) {
        this->globaltransport_beatTimeChanges[0] = this->globaltransport_beatTimeChanges[(Index)(this->globaltransport_beatTimeChanges->length - 2)];
        this->globaltransport_beatTimeChanges[1] = this->globaltransport_beatTimeChanges[(Index)(this->globaltransport_beatTimeChanges->length - 1)];
        this->globaltransport_beatTimeChanges->length = 2;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendBeatTimeEvent(this->globaltransport_beatTimeChanges[0]);
        }
    }

    if (this->globaltransport_timeSignatureChanges->length > 3) {
        this->globaltransport_timeSignatureChanges[0] = this->globaltransport_timeSignatureChanges[(Index)(this->globaltransport_timeSignatureChanges->length - 3)];
        this->globaltransport_timeSignatureChanges[1] = this->globaltransport_timeSignatureChanges[(Index)(this->globaltransport_timeSignatureChanges->length - 2)];
        this->globaltransport_timeSignatureChanges[2] = this->globaltransport_timeSignatureChanges[(Index)(this->globaltransport_timeSignatureChanges->length - 1)];
        this->globaltransport_timeSignatureChanges->length = 3;

        if ((bool)(this->globaltransport_notify)) {
            this->getEngine()->sendTimeSignatureEvent(
                (Int)(this->globaltransport_timeSignatureChanges[0]),
                (Int)(this->globaltransport_timeSignatureChanges[1])
            );
        }
    }

    this->globaltransport_notify = false;
}

void globaltransport_dspsetup(bool force) {
    if ((bool)(this->globaltransport_setupDone) && (bool)(!(bool)(force)))
        return;

    fillSignal(this->globaltransport_tempo, this->vs, this->globaltransport_lastTempo);
    this->globaltransport_tempoNeedsReset = false;
    fillSignal(this->globaltransport_state, this->vs, this->globaltransport_lastState);
    this->globaltransport_stateNeedsReset = false;
    this->globaltransport_setupDone = true;
}

bool stackprotect_check() {
    this->stackprotect_count++;

    if (this->stackprotect_count > 128) {
        console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
        return true;
    }

    return false;
}

Index getPatcherSerial() const {
    return 0;
}

void sendParameter(ParameterIndex index, bool ignoreValue) {
    this->getEngine()->notifyParameterValueChanged(index, (ignoreValue ? 0 : this->getParameterValue(index)), ignoreValue);
}

void scheduleParamInit(ParameterIndex index, Index order) {
    this->paramInitIndices->push(index);
    this->paramInitOrder->push(order);
}

void processParamInitEvents() {
    this->listquicksort(
        this->paramInitOrder,
        this->paramInitIndices,
        0,
        (int)(this->paramInitOrder->length - 1),
        true
    );

    for (Index i = 0; i < this->paramInitOrder->length; i++) {
        this->getEngine()->scheduleParameterBang(this->paramInitIndices[i], 0);
    }
}

void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
    RNBO_UNUSED(inProcess);
    RNBO_UNUSED(engine);
    this->_currentTime = time;
    auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));

    if (offset >= (SampleIndex)(this->vs))
        offset = (SampleIndex)(this->vs) - 1;

    if (offset < 0)
        offset = 0;

    this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
}

void assign_defaults()
{
    midiin_port = 0;
    midiout_port = 0;
    param_19_value = 1;
    param_20_value = 12000;
    param_21_value = 1;
    param_22_value = 1;
    param_23_value = 0;
    param_24_value = 0.2;
    poly_target = 0;
    poly_midiin = 0;
    midinotecontroller_01_currenttarget = 0;
    midinotecontroller_01_midiin = 0;
    ctlin_17_input = 0;
    ctlin_17_controller = 0;
    ctlin_17_channel = -1;
    expr_19_in1 = 0;
    expr_19_in2 = 0.007874015748;
    expr_19_out1 = 0;
    ctlin_18_input = 0;
    ctlin_18_controller = 0;
    ctlin_18_channel = -1;
    expr_20_in1 = 0;
    expr_20_in2 = 0.007874015748;
    expr_20_out1 = 0;
    ctlin_19_input = 0;
    ctlin_19_controller = 0;
    ctlin_19_channel = -1;
    expr_21_in1 = 0;
    expr_21_in2 = 0.007874015748;
    expr_21_out1 = 0;
    ctlin_20_input = 0;
    ctlin_20_controller = 0;
    ctlin_20_channel = -1;
    expr_22_in1 = 0;
    expr_22_in2 = 0.007874015748;
    expr_22_out1 = 0;
    _currentTime = 0;
    audioProcessSampleCount = 0;
    sampleOffsetIntoNextAudioBuffer = 0;
    zeroBuffer = nullptr;
    dummyBuffer = nullptr;
    didAllocateSignals = 0;
    vs = 0;
    maxvs = 0;
    sr = 44100;
    invsr = 0.000022675736961451248;
    midiout_currentStatus = -1;
    midiout_status = -1;
    midiout_byte1 = -1;
    param_19_lastValue = 0;
    param_20_lastValue = 0;
    param_21_lastValue = 0;
    param_22_lastValue = 0;
    param_23_lastValue = 0;
    param_24_lastValue = 0;
    poly_currentStatus = -1;
    poly_mididata[0] = 0;
    poly_mididata[1] = 0;
    poly_mididata[2] = 0;
    midinotecontroller_01_currentStatus = -1;
    midinotecontroller_01_status = -1;
    midinotecontroller_01_byte1 = -1;
    midinotecontroller_01_zone_masterchannel = 1;
    midinotecontroller_01_zone_numnotechannels = 15;
    midinotecontroller_01_zone_masterpitchbendrange = 2;
    midinotecontroller_01_zone_pernotepitchbendrange = 48;
    midinotecontroller_01_muteval = { 0, 0 };
    ctlin_17_status = 0;
    ctlin_17_byte1 = -1;
    ctlin_17_inchan = 0;
    ctlin_18_status = 0;
    ctlin_18_byte1 = -1;
    ctlin_18_inchan = 0;
    ctlin_19_status = 0;
    ctlin_19_byte1 = -1;
    ctlin_19_inchan = 0;
    ctlin_20_status = 0;
    ctlin_20_byte1 = -1;
    ctlin_20_inchan = 0;
    globaltransport_tempo = nullptr;
    globaltransport_tempoNeedsReset = false;
    globaltransport_lastTempo = 120;
    globaltransport_state = nullptr;
    globaltransport_stateNeedsReset = false;
    globaltransport_lastState = 0;
    globaltransport_beatTimeChanges = { 0, 0 };
    globaltransport_timeSignatureChanges = { 4, 4, 0 };
    globaltransport_notify = false;
    globaltransport_bbuBase = 0;
    globaltransport_setupDone = false;
    stackprotect_count = 0;
    _voiceIndex = 0;
    _noteNumber = 0;
    isMuted = 1;
}

    // data ref strings
    struct DataRefStrings {
    	static constexpr auto& name0 = "RNBODefaultSinus";
    	static constexpr auto& file0 = "";
    	static constexpr auto& tag0 = "buffer~";
    	static constexpr auto& name1 = "RNBODefaultMtofLookupTable256";
    	static constexpr auto& file1 = "";
    	static constexpr auto& tag1 = "buffer~";
    	DataRefStrings* operator->() { return this; }
    	const DataRefStrings* operator->() const { return this; }
    };

    DataRefStrings dataRefStrings;

// member variables

    number midiin_port;
    number midiout_port;
    number param_19_value;
    number param_20_value;
    number param_21_value;
    number param_22_value;
    number param_23_value;
    number param_24_value;
    number poly_target;
    number poly_midiin;
    number midinotecontroller_01_currenttarget;
    number midinotecontroller_01_midiin;
    number ctlin_17_input;
    number ctlin_17_controller;
    number ctlin_17_channel;
    number expr_19_in1;
    number expr_19_in2;
    number expr_19_out1;
    number ctlin_18_input;
    number ctlin_18_controller;
    number ctlin_18_channel;
    number expr_20_in1;
    number expr_20_in2;
    number expr_20_out1;
    number ctlin_19_input;
    number ctlin_19_controller;
    number ctlin_19_channel;
    number expr_21_in1;
    number expr_21_in2;
    number expr_21_out1;
    number ctlin_20_input;
    number ctlin_20_controller;
    number ctlin_20_channel;
    number expr_22_in1;
    number expr_22_in2;
    number expr_22_out1;
    MillisecondTime _currentTime;
    ENGINE _internalEngine;
    UInt64 audioProcessSampleCount;
    Index sampleOffsetIntoNextAudioBuffer;
    signal zeroBuffer;
    signal dummyBuffer;
    bool didAllocateSignals;
    Index vs;
    Index maxvs;
    number sr;
    number invsr;
    Int midiout_currentStatus;
    Int midiout_status;
    Int midiout_byte1;
    list midiout_sysex;
    number param_19_lastValue;
    number param_20_lastValue;
    number param_21_lastValue;
    number param_22_lastValue;
    number param_23_lastValue;
    number param_24_lastValue;
    Int poly_currentStatus;
    uint8_t poly_mididata[3];
    Int midinotecontroller_01_currentStatus;
    Int midinotecontroller_01_status;
    Int midinotecontroller_01_byte1;
    Int midinotecontroller_01_zone_masterchannel;
    Int midinotecontroller_01_zone_numnotechannels;
    Int midinotecontroller_01_zone_masterpitchbendrange;
    Int midinotecontroller_01_zone_pernotepitchbendrange;
    number midinotecontroller_01_channel_pitchbend[16] = { };
    number midinotecontroller_01_channel_pressure[16] = { };
    number midinotecontroller_01_channel_timbre[16] = { };
    Int midinotecontroller_01_channel_sustain[16] = { };
    MillisecondTime midinotecontroller_01_voice_lastontime[4] = { };
    number midinotecontroller_01_voice_notenumber[4] = { };
    number midinotecontroller_01_voice_channel[4] = { };
    number midinotecontroller_01_voice_state[4] = { };
    number midinotecontroller_01_notesdown[129] = { };
    number midinotecontroller_01_note_lastvelocity[128] = { };
    list midinotecontroller_01_muteval;
    Int ctlin_17_status;
    Int ctlin_17_byte1;
    Int ctlin_17_inchan;
    Int ctlin_18_status;
    Int ctlin_18_byte1;
    Int ctlin_18_inchan;
    Int ctlin_19_status;
    Int ctlin_19_byte1;
    Int ctlin_19_inchan;
    Int ctlin_20_status;
    Int ctlin_20_byte1;
    Int ctlin_20_inchan;
    signal globaltransport_tempo;
    bool globaltransport_tempoNeedsReset;
    number globaltransport_lastTempo;
    signal globaltransport_state;
    bool globaltransport_stateNeedsReset;
    number globaltransport_lastState;
    list globaltransport_beatTimeChanges;
    list globaltransport_timeSignatureChanges;
    bool globaltransport_notify;
    number globaltransport_bbuBase;
    bool globaltransport_setupDone;
    number stackprotect_count;
    DataRef RNBODefaultSinus;
    DataRef RNBODefaultMtofLookupTable256;
    Index _voiceIndex;
    Int _noteNumber;
    Index isMuted;
    indexlist paramInitIndices;
    indexlist paramInitOrder;
    RNBOSubpatcher_953 poly[4];
    bool _isInitialized = false;
};

static PatcherInterface* creaternbomatic()
{
    return new rnbomatic<EXTERNALENGINE>();
}

#ifndef RNBO_NO_PATCHERFACTORY
extern "C" PatcherFactoryFunctionPtr GetPatcherFactoryFunction()
#else
extern "C" PatcherFactoryFunctionPtr rnbomaticFactoryFunction()
#endif
{
    return creaternbomatic;
}

#ifndef RNBO_NO_PATCHERFACTORY
extern "C" void SetLogger(Logger* logger)
#else
void rnbomaticSetLogger(Logger* logger)
#endif
{
    console = logger;
}

} // end RNBO namespace

