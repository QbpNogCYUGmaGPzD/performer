#pragma once

#include "Config.h"
#include "Bitfield.h"
#include "Serialize.h"
#include "ModelUtils.h"
#include "Types.h"
#include "Scale.h"
#include "Routing.h"

#include "core/math/Math.h"
#include "core/utils/StringBuilder.h"

#include <array>
#include <cstdint>
#include <initializer_list>

class NoteSequence {
public:
    //----------------------------------------
    // Types
    //----------------------------------------

    typedef UnsignedValue<3> GateProbability;
    typedef UnsignedValue<2> Retrigger;
    typedef UnsignedValue<3> RetriggerProbability;
    typedef UnsignedValue<3> Length;
    typedef SignedValue<4> LengthVariationRange;
    typedef UnsignedValue<3> LengthVariationProbability;
    typedef SignedValue<7> Note;
    typedef SignedValue<7> NoteVariationRange;
    typedef UnsignedValue<3> NoteVariationProbability;

    enum class Layer {
        Gate,
        GateProbability,
        Retrigger,
        RetriggerProbability,
        Length,
        LengthVariationRange,
        LengthVariationProbability,
        Note,
        NoteVariationRange,
        NoteVariationProbability,
        Slide,
        Last
    };

    static const char *layerName(Layer layer) {
        switch (layer) {
        case Layer::Gate:                       return "GATE";
        case Layer::GateProbability:            return "GATE PROB";
        case Layer::Retrigger:                  return "RETRIG";
        case Layer::RetriggerProbability:       return "RETRIG PROB";
        case Layer::Length:                     return "LENGTH";
        case Layer::LengthVariationRange:       return "LENGTH RANGE";
        case Layer::LengthVariationProbability: return "LENGTH PROB";
        case Layer::Note:                       return "NOTE";
        case Layer::NoteVariationRange:         return "NOTE RANGE";
        case Layer::NoteVariationProbability:   return "NOTE PROB";
        case Layer::Slide:                      return "SLIDE";
        case Layer::Last:                       break;
        }
        return nullptr;
    }

    static Types::LayerRange layerRange(Layer layer);

    class Step {
    public:
        // gate

        bool gate() const { return _data0.gate ? true : false; }
        void setGate(bool gate) { _data0.gate = gate; }
        void toggleGate() { setGate(!gate()); }

        // gateProbability

        int gateProbability() const { return _data0.gateProbability; }
        void setGateProbability(int gateProbability) {
            _data0.gateProbability = GateProbability::clamp(gateProbability);
        }

        // slide

        bool slide() const { return _data0.slide ? true : false; }
        void setSlide(bool slide) {
            _data0.slide = slide;
        }
        void toggleSlide() {
            setSlide(!slide());
        }

        // retrigger

        int retrigger() const { return _data1.retrigger; }
        void setRetrigger(int retrigger) {
            _data1.retrigger = Retrigger::clamp(retrigger);
        }

        // retriggerProbability

        int retriggerProbability() const { return _data1.retriggerProbability; }
        void setRetriggerProbability(int retriggerProbability) {
            _data1.retriggerProbability = RetriggerProbability::clamp(retriggerProbability);
        }

        // length

        int length() const { return _data0.length; }
        void setLength(int length) {
            _data0.length = Length::clamp(length);
        }

        // lengthVariationRange

        int lengthVariationRange() const { return LengthVariationRange::Min + _data0.lengthVariationRange; }
        void setLengthVariationRange(int lengthVariationRange) {
            _data0.lengthVariationRange = LengthVariationRange::clamp(lengthVariationRange) - LengthVariationRange::Min;
        }

        // lengthVariationProbability

        int lengthVariationProbability() const { return _data0.lengthVariationProbability; }
        void setLengthVariationProbability(int lengthVariationProbability) {
            _data0.lengthVariationProbability = LengthVariationProbability::clamp(lengthVariationProbability);
        }

        // note

        int note() const { return Note::Min + _data0.note; }
        void setNote(int note) {
            _data0.note = Note::clamp(note) - Note::Min;
        }

        // noteVariationRange

        int noteVariationRange() const { return NoteVariationRange::Min + _data0.noteVariationRange; }
        void setNoteVariationRange(int noteVariationRange) {
            _data0.noteVariationRange = NoteVariationRange::clamp(noteVariationRange) - NoteVariationRange::Min;
        }

        // noteVariationProbability

        int noteVariationProbability() const { return _data0.noteVariationProbability; }
        void setNoteVariationProbability(int noteVariationProbability) {
            _data0.noteVariationProbability = NoteVariationProbability::clamp(noteVariationProbability);
        }

        int layerValue(Layer layer) const;
        void setLayerValue(Layer layer, int value);

        void clear();

        void write(WriteContext &context) const;
        void read(ReadContext &context);

    private:
        union {
            uint32_t raw;
            BitField<uint32_t, 0, 1> gate;
            BitField<uint32_t, 1, 1> slide;
            BitField<uint32_t, 2, GateProbability::Bits> gateProbability;
            BitField<uint32_t, 5, Length::Bits> length;
            BitField<uint32_t, 8, LengthVariationRange::Bits> lengthVariationRange;
            BitField<uint32_t, 12, LengthVariationProbability::Bits> lengthVariationProbability;
            BitField<uint32_t, 15, Note::Bits> note;
            BitField<uint32_t, 22, NoteVariationRange::Bits> noteVariationRange;
            BitField<uint32_t, 29, NoteVariationProbability::Bits> noteVariationProbability;
        } _data0;
        union {
            uint16_t raw;
            BitField<uint16_t, 0, Retrigger::Bits> retrigger;
            BitField<uint16_t, 2, RetriggerProbability::Bits> retriggerProbability;
            // 11 bits left
        } _data1;
    };

    typedef std::array<Step, CONFIG_STEP_COUNT> StepArray;

    //----------------------------------------
    // Properties
    //----------------------------------------

    // scale

    int scale() const { return _scale; }
    void setScale(int scale) {
        _scale = clamp(scale, -1, Scale::Count - 1);
    }

    int indexedScale() const { return _scale + 1; }
    void setIndexedScale(int index) {
        setScale(index - 1);
    }

    void editScale(int value, bool shift) {
        setScale(scale() + value);
    }

    void printScale(StringBuilder &str) const {
        str(scale() < 0 ? "Default" : Scale::name(scale()));
    }

    const Scale &selectedScale(int defaultScale) const {
        return Scale::get(scale() < 0 ? defaultScale : scale());
    }

    // rootNote

    int rootNote() const { return _rootNote; }
    void setRootNote(int rootNote) {
        _rootNote = clamp(rootNote, -1, 11);
    }

    int indexedRootNote() const { return _rootNote + 1; }
    void setIndexedRootNote(int index) {
        setRootNote(index - 1);
    }

    void editRootNote(int value, bool shift) {
        setRootNote(rootNote() + value);
    }

    void printRootNote(StringBuilder &str) const {
        if (rootNote() < 0) {
            str("Default");
        } else {
            Types::printNote(str, rootNote());
        }
    }

    int selectedRootNote(int defaultRootNote) const {
        return rootNote() < 0 ? defaultRootNote : rootNote();
    }

    // divisor

    int divisor() const { return _divisor; }
    void setDivisor(int divisor) {
        _divisor = clamp(divisor, 1, 192);
    }

    int indexedDivisor() const { return ModelUtils::divisorToIndex(divisor()); }
    void setIndexedDivisor(int index) {
        int divisor = ModelUtils::indexToDivisor(index);
        if (divisor > 0) {
            setDivisor(divisor);
        }
    }

    void editDivisor(int value, bool shift) {
        setDivisor(ModelUtils::adjustedByDivisor(divisor(), value, shift));
    }

    void printDivisor(StringBuilder &str) const {
        ModelUtils::printDivisor(str, divisor());
    }

    // resetMeasure

    int resetMeasure() const { return _resetMeasure; }
    void setResetMeasure(int resetMeasure) {
        _resetMeasure = clamp(resetMeasure, 0, 128);
    }

    void editResetMeasure(int value, bool shift) {
        setResetMeasure(ModelUtils::adjustedByPowerOfTwo(resetMeasure(), value, shift));
    }

    void printResetMeasure(StringBuilder &str) const {
        if (resetMeasure() == 0) {
            str("off");
        } else {
            str("%d", resetMeasure());
        }
    }

    // runMode

    Types::RunMode runMode() const { return _runMode.get(isRouted(Routing::Target::RunMode)); }
    void setRunMode(Types::RunMode runMode, bool routed = false) {
        _runMode.set(ModelUtils::clampedEnum(runMode), routed);
    }

    void editRunMode(int value, bool shift) {
        if (!isRouted(Routing::Target::RunMode)) {
            setRunMode(ModelUtils::adjustedEnum(runMode(), value));
        }
    }

    void printRunMode(StringBuilder &str) const {
        _routed.print(str, Routing::Target::RunMode);
        str(Types::runModeName(runMode()));
    }

    // firstStep

    int firstStep() const { return _firstStep.get(isRouted(Routing::Target::FirstStep)); }
    void setFirstStep(int firstStep, bool routed = false) {
        _firstStep.set(clamp(firstStep, 0, lastStep()), routed);
    }

    void editFirstStep(int value, bool shift) {
        if (!isRouted(Routing::Target::FirstStep)) {
            setFirstStep(firstStep() + value);
        }
    }

    void printFirstStep(StringBuilder &str) const {
        _routed.print(str, Routing::Target::FirstStep);
        str("%d", firstStep() + 1);
    }

    // lastStep

    int lastStep() const { return _lastStep.get(isRouted(Routing::Target::LastStep)); }
    void setLastStep(int lastStep, bool routed = false) {
        _lastStep.set(clamp(lastStep, firstStep(), CONFIG_STEP_COUNT - 1), routed);
    }

    void editLastStep(int value, bool shift) {
        if (!isRouted(Routing::Target::LastStep)) {
            setLastStep(lastStep() + value);
        }
    }

    void printLastStep(StringBuilder &str) const {
        _routed.print(str, Routing::Target::LastStep);
        str("%d", lastStep() + 1);
    }

    // steps

    const StepArray &steps() const { return _steps; }
          StepArray &steps()       { return _steps; }

    const Step &step(int index) const { return _steps[index]; }
          Step &step(int index)       { return _steps[index]; }

    //----------------------------------------
    // Routing
    //----------------------------------------

    inline bool isRouted(Routing::Target target) const { return _routed.has(target); }
    inline void setRouted(Routing::Target target, bool routed) { _routed.set(target, routed); }
    void writeRouted(Routing::Target target, int intValue, float floatValue);

    //----------------------------------------
    // Methods
    //----------------------------------------

    void clear();
    void clearSteps();

    void setGates(std::initializer_list<int> gates);
    void setNotes(std::initializer_list<int> notes);

    void shiftSteps(int direction);

    void duplicateSteps();

    void write(WriteContext &context) const;
    void read(ReadContext &context);

private:
    int8_t _scale;
    int8_t _rootNote;
    uint8_t _divisor;
    uint8_t _resetMeasure;
    Routable<Types::RunMode> _runMode;
    Routable<uint8_t> _firstStep;
    Routable<uint8_t> _lastStep;

    RoutableSet<Routing::Target::SequenceFirst, Routing::Target::SequenceLast> _routed;

    StepArray _steps;

    friend class Project;
};
