//-----------------------------------------------------------------------------
// Filename: OscilliscopeVisualiser.cs
//
// Description: Draws a bitmap to visualise audio as an oscilloscope output.

// Author(s):
// Aaron Clauson (aaron@sipsorcery.com)
//
// History:
// 29 Feb 2020	Aaron Clauson	Created, Dublin, Ireland.
//
// License: 
// BSD 3-Clause "New" or "Revised" License, see included LICENSE.md file.
//-----------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Text;

namespace HilbertAudioVisualiser
{
    public class OscilliscopeVisualiser
    {
        /* starter: draw a line */
        // ctx.beginPath()
        // ctx.moveTo(0, 0)
        // ctx.lineTo(640, 360)
        // ctx.stroke()

        /* naive oscilloscope */
        // ctx.beginPath()
        // for (let i = 0; i < bufferLength; i++) {
        //   // get sample
        //   const v = waveformData[i]
        //   // normalize sample to range of 0 to 1
        //   const vn = v * 0.5 + 0.5

        //   // get x and y position of the sample
        //   // n - 1 because last sample index is n - 1
        //   const x = i / (bufferLength - 1) * canvas.width
        //   // 1 - value because y is down
        //   const y = (1 - vn) * canvas.height

        //   if (i === 0) {
        //     // move to sample
        //     ctx.moveTo(x, y)
        //   } else {
        //     // draw line to sample
        //     ctx.lineTo(x, y)
        //   }
        // }
        // ctx.stroke()
    }
}
