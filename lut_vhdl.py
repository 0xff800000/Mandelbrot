iterations = 128

def sweep_color(val):
    r,g,b = (0,0,0)
    if val == iterations: return (r,g,b)
    wavelength = (iterations-val)*(750-380)/iterations+380
    attenuation = 0

    if wavelength >= 380 and wavelength <=440:
        attenuation = 0.3 + 0.7 * (wavelength - 380) / (440 - 380)
        r = 0xff * ((-(wavelength-440)/(440-380)) * attenuation)
        b = 0xff * attenuation

    elif wavelength >= 440 and wavelength <= 490:
        g = 0xff * ((wavelength - 440)/(490 - 440))
        b = 0xff

    elif wavelength >= 490 and wavelength <= 510:
        g = 0xff
        b = 0xff * (-(wavelength - 510) / (510 - 490))

    elif wavelength >= 510 and wavelength <= 580:
        r = 0xff * ((wavelength - 510) / (580 - 510))
        g = 0xff

    elif wavelength >= 580 and wavelength <= 645:
        r = 0xff
        g = 0xff * (-(wavelength - 645) / (645 - 580))

    elif wavelength >= 645 and wavelength <= 750:
        attenuation = 0.3 + 0.7 * (750 - wavelength) / (750 - 645)
        r = 0xff * attenuation

    return (int(r),int(g),int(b))

for i in range(0,127):
    color = sweep_color(i)
    color_str = '{:02x}{:02x}{:02x}'.format(color[0],color[1],color[2])
    print('x\"{}\" when iteration_s = {:04x} else'.format(color_str,i))
