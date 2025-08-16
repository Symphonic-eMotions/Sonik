//
//  RNBOAudioKitMIDIRouter.swift
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Frans-Jan Wind on 17/07/2025.
//

import AudioKit
import CoreMIDI

class RNBOAudioKitMIDIRouter: NSObject, MIDIListener {
    private let rnboHostModel: RNBOAudioUnitHostModel

    init(rnboHostModel: RNBOAudioUnitHostModel) {
        self.rnboHostModel = rnboHostModel
        super.init()
        
        // Open MIDI‑input en registreer self als listener
        MIDI.sharedInstance.openInput()
        MIDI.sharedInstance.addListener(self)
    }

    // MARK: – MIDIListener conforming

    func receivedMIDINoteOn(noteNumber: MIDINoteNumber,
                            velocity: MIDIVelocity,
                            channel: MIDIChannel,
                            portID: MIDIUniqueID?,
                            timeStamp: MIDITimeStamp?) {
        // stuur door naar RNBO
        rnboHostModel.sendNoteOn(noteNumber, velocity: velocity, channel: channel)
    }

    func receivedMIDINoteOff(noteNumber: MIDINoteNumber,
                             velocity: MIDIVelocity,
                             channel: MIDIChannel,
                             portID: MIDIUniqueID?,
                             timeStamp: MIDITimeStamp?) {
        // stuur door naar RNBO
        rnboHostModel.sendNoteOff(noteNumber, releaseVelocity: velocity, channel: channel)
    }

    func receivedMIDIController(_ controller: MIDIByte,
                                value: MIDIByte,
                                channel: MIDIChannel,
                                portID: MIDIUniqueID?,
                                timeStamp: MIDITimeStamp?) {
        // stuur CC's door naar RNBO
        rnboHostModel.sendContinuousController(controller, value: value, channel: channel)
    }

    // ————————————————————————————————————————————————————————————————————————————
    // De volgende methodes zijn verplicht volgens het MIDIListener‑protocol
    // ————————————————————————————————————————————————————————————————————————————

    func receivedMIDIAftertouch(noteNumber: MIDINoteNumber,
                                pressure: MIDIByte,
                                channel: MIDIChannel,
                                portID: MIDIUniqueID?,
                                timeStamp: MIDITimeStamp?) { }

    func receivedMIDIAftertouch(_ pressure: MIDIByte,
                                channel: MIDIChannel,
                                portID: MIDIUniqueID?,
                                timeStamp: MIDITimeStamp?) { }

    func receivedMIDIPitchWheel(_ pitchWheelValue: MIDIWord,
                                channel: MIDIChannel,
                                portID: MIDIUniqueID?,
                                timeStamp: MIDITimeStamp?) { }

    func receivedMIDIProgramChange(_ program: MIDIByte,
                                   channel: MIDIChannel,
                                   portID: MIDIUniqueID?,
                                   timeStamp: MIDITimeStamp?) { }

    func receivedMIDISystemExclusive(_ data: [MIDIByte],
                                     portID: MIDIUniqueID?,
                                     timeStamp: MIDITimeStamp?) { }

    func receivedMIDISetupChange() { }

    func receivedMIDIPropertyChange(propertyChangeInfo: MIDIObjectPropertyChangeNotification) { }

    func receivedMIDINotification(notification: MIDINotification) { }
    
    func receivedMIDIChannelAftertouch(_ pressure: MIDIByte,
                                       channel: MIDIChannel,
                                       portID: MIDIUniqueID?,
                                       timeStamp: MIDITimeStamp?) { }
    
    func receivedMIDISystemCommand(_ data: [MIDIByte],
                                   portID: MIDIUniqueID?,
                                   timeStamp: MIDITimeStamp?) { }
        
    func receivedMIDITimecodeQuarterFrame(_ quarterFrame: MIDIByte,
                                          portID: MIDIUniqueID?,
                                          timeStamp: MIDITimeStamp?) { }
    
    func receivedMIDISongPosition(_ songPosition: MIDIWord,
                                  portID: MIDIUniqueID?,
                                  timeStamp: MIDITimeStamp?) { }
    
    func receivedMIDISongSelect(_ songSelect: MIDIByte,
                                portID: MIDIUniqueID?,
                                timeStamp: MIDITimeStamp?) { }
    
    func receivedMIDITuneRequest(portID: MIDIUniqueID?,
                                 timeStamp: MIDITimeStamp?) { }
    
    func receivedMIDITiming(portID: MIDIUniqueID?,
                            timeStamp: MIDITimeStamp?) { }
    
    func receivedMIDIStart(portID: MIDIUniqueID?,
                           timeStamp: MIDITimeStamp?) { }
    
    func receivedMIDIContinue(portID: MIDIUniqueID?,
                              timeStamp: MIDITimeStamp?) { }
    
    func receivedMIDIStop(portID: MIDIUniqueID?,
                          timeStamp: MIDITimeStamp?) { }
    
    func receivedMIDIActiveSensing(portID: MIDIUniqueID?,
                                   timeStamp: MIDITimeStamp?) { }
    
    func receivedMIDISystemReset(portID: MIDIUniqueID?,
                                 timeStamp: MIDITimeStamp?) { }
}
