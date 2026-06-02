// AstroDefender polyglot module: Go telemetry_service
// ASTRO_POLYGLOT_CONTRACT_VERSION: 1.0.0
// Packages shared runtime snapshots for leaderboard/analytics services.
package astrotelemetry

import "math"

const ContractVersion = "1.0.0"
const LogicalWidth = 960
const LogicalHeight = 720

type FrameSnapshot struct {
	Tick            int    `json:"tick"`
	Score           int    `json:"score"`
	Level           int    `json:"level"`
	ContractVersion string `json:"contract_version"`
}

type Viewport struct { X, Y, Width, Height int; Scale float64 }

func CalculateViewport(screenWidth, screenHeight int) Viewport {
	scale := math.Min(float64(screenWidth)/LogicalWidth, float64(screenHeight)/LogicalHeight)
	width := int(math.Round(LogicalWidth * scale))
	height := int(math.Round(LogicalHeight * scale))
	return Viewport{X: (screenWidth - width) / 2, Y: (screenHeight - height) / 2, Width: width, Height: height, Scale: scale}
}

func NewSnapshot(tick, score, level int) FrameSnapshot {
	return FrameSnapshot{Tick: tick, Score: score, Level: level, ContractVersion: ContractVersion}
}
