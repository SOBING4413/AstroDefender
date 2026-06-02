// Go + Ebiten: return scale and offset for drawing a 960x720 logical image.
package aspectfit

import "math"

const LogicalWidth = 960
const LogicalHeight = 720

type Viewport struct { X, Y, Width, Height int; Scale float64 }

func Calculate(screenWidth, screenHeight int) Viewport {
    scale := math.Min(float64(screenWidth)/LogicalWidth, float64(screenHeight)/LogicalHeight)
    width := int(math.Round(LogicalWidth * scale))
    height := int(math.Round(LogicalHeight * scale))
    return Viewport{X: (screenWidth - width) / 2, Y: (screenHeight - height) / 2, Width: width, Height: height, Scale: scale}
}
