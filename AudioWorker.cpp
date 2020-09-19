#include "AudioWorker.h"
#include <OpenCL/opencl.h>
#include <QDebug>
#include <QFile>
#include <QtMath>
#include <cstring>
#include <thread>
#include <AudioToolbox/AudioToolbox.h>

#define BUFFER_SIZE 1024*2

struct SoundState
{
    float angle;

    cl_device_id device_id;
    cl_context context;
    cl_command_queue commands;
    cl_program program;
    cl_kernel kernel;
    cl_mem theta;
    cl_mem output;
    int err;
};
SoundState soundState {};

float *song;


void AudioWorker::setupCL()
{
    // Read kernel source
    QByteArray source;
    QFile f(":/res/program.cl");
    if(f.open(QIODevice::ReadOnly))
    {
        source = f.readAll();
        f.close();
    }
    const char * programSource = source.constData();

    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &soundState.device_id, NULL);
    soundState.context = clCreateContext(0, 1, &soundState.device_id, NULL, NULL, &soundState.err);
    soundState.commands = clCreateCommandQueue(soundState.context, soundState.device_id, 0, &soundState.err);
    soundState.program = clCreateProgramWithSource(soundState.context, 1, (const char **) &programSource, NULL, &soundState.err);

    clBuildProgram(soundState.program, 0, NULL, NULL, NULL, &soundState.err);

    soundState.kernel = clCreateKernel(soundState.program, "toneGenerator", &soundState.err);
    soundState.output = clCreateBuffer(soundState.context, CL_MEM_WRITE_ONLY, BUFFER_SIZE, NULL, NULL);
    soundState.theta = clCreateBuffer(soundState.context, CL_MEM_READ_WRITE, sizeof(float), NULL, NULL);

    // Sets the buffer argument
    clSetKernelArg(soundState.kernel, 0, sizeof(cl_mem), &soundState.output);

    // Sets the buffer length argument
    uint bufferLenght = BUFFER_SIZE/4;
    clSetKernelArg(soundState.kernel, 1, sizeof(unsigned int), &bufferLenght);

    // Sets the frame rate argument
    uint frameRate = 44100;
    clSetKernelArg(soundState.kernel, 2, sizeof(unsigned int), &frameRate);

    // Sets the frame rate argument
    clSetKernelArg(soundState.kernel, 3, sizeof(cl_mem), &soundState.theta);

    // Sets the tone frecuency argument
    uint toneFreq = 440;
    clSetKernelArg(soundState.kernel, 4, sizeof(unsigned int), &toneFreq);

    // Sets the volume
    float volume = 1.f;
    clSetKernelArg(soundState.kernel, 5, sizeof(float), &volume);


    // Sets the initial theta angle
    clEnqueueWriteBuffer(
                soundState.commands,
                soundState.theta,
                CL_TRUE,
                0,
                sizeof(float),
                &soundState.angle,
                0,
                NULL,
                NULL);


}

OSStatus AudioCallback(
    void *inRefCon,
    AudioUnitRenderActionFlags *ioActionFlags,
    const AudioTimeStamp *inTimeStamp,
    UInt32 inBusNumber,
    UInt32 inNumberFrames,
    AudioBufferList *ioData)

{
    Q_UNUSED(ioActionFlags);
    Q_UNUSED(inTimeStamp);
    Q_UNUSED(inBusNumber);
    Q_UNUSED(inNumberFrames);

    SoundState *soundState = (SoundState*)inRefCon;

    // To use only 1 GPU Unit
    size_t local = 1;
    size_t global = 1;
    clEnqueueNDRangeKernel(soundState->commands, soundState->kernel, 1, NULL, &global, &local, 0, NULL, NULL);

    // Run kernel
    clFinish(soundState->commands);

    // Reads the audio output
    float *buffer = (float *)ioData->mBuffers[0].mData;
    clEnqueueReadBuffer(soundState->commands, soundState->output, CL_TRUE, 0, BUFFER_SIZE, buffer, 0, NULL, NULL);

    return noErr;
}




void AudioWorker::runAudio()
{

    setupCL();

    AudioUnit toneUnit;

    // The output audio unit description
    AudioComponentDescription defaultOutputDescription;
    defaultOutputDescription.componentType = kAudioUnitType_Output;
    defaultOutputDescription.componentSubType = kAudioUnitSubType_DefaultOutput;
    defaultOutputDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
    defaultOutputDescription.componentFlags = 0;
    defaultOutputDescription.componentFlagsMask = 0;

    // Get the default playback output unit
    AudioComponent defaultOutput = AudioComponentFindNext(NULL, &defaultOutputDescription);
    AudioComponentInstanceNew(defaultOutput, &toneUnit);

    // User data
    soundState.angle = 0.0f;

    // Set the audio callback function
    AURenderCallbackStruct input;
    input.inputProc = AudioCallback;
    input.inputProcRefCon = &soundState;

    AudioUnitSetProperty(toneUnit,
        kAudioUnitProperty_SetRenderCallback,
        kAudioUnitScope_Input,
        0,
        &input,
        sizeof(input));

    // Define 44.1kh mono float audio
    AudioStreamBasicDescription auDesc {};
    auDesc.mSampleRate = 44100;
    auDesc.mFormatID = kAudioFormatLinearPCM;
    auDesc.mFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
    auDesc.mBytesPerPacket = 4;
    auDesc.mFramesPerPacket = 1;
    auDesc.mBytesPerFrame = 4;
    auDesc.mChannelsPerFrame = 1;
    auDesc.mBitsPerChannel = 32;

    AudioUnitSetProperty (toneUnit,
        kAudioUnitProperty_StreamFormat,
        kAudioUnitScope_Input,
        0,
        &auDesc,
        sizeof(AudioStreamBasicDescription));

    // Number of frames (4 bytes each)
    UInt32 buffSize = BUFFER_SIZE/4;

    AudioUnitSetProperty(toneUnit,
        kAudioDevicePropertyBufferFrameSize,
        kAudioUnitScope_Global,
        0,
        &buffSize, sizeof(UInt32));

    // Playback audio
    AudioUnitInitialize(toneUnit);
    AudioOutputUnitStart(toneUnit);

}

void AudioWorker::volumeChanged(int val)
{
    // Sets the volume
    float volume = float(val)/1024.f;
    clSetKernelArg(soundState.kernel, 5, sizeof(float), &volume);
}

void AudioWorker::freqChanged(int val)
{
    // Sets the frame rate argument
    uint toneFreq = (uint)val;
    clSetKernelArg(soundState.kernel, 4, sizeof(unsigned int), &toneFreq);
}
