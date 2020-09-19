kernel void toneGenerator(
    global float *output,
    const unsigned int bufferLenght,
    const unsigned int frameRate,
    global float *theta,
    const unsigned int toneFreq,
    const float volume)
{
    float pi = 3.141592653589793115998;
    float theta_increment = 2.0f * pi * float(toneFreq) / float(frameRate);

    for (unsigned int frame = 0; frame < bufferLenght; frame++)
    {
        output[frame] = sin(theta[0]) * volume;

        theta[0] += theta_increment;
        if (theta[0] > 2.0 * pi)
        {
            theta[0] -= 2.0 * pi;
        }
    }

}

