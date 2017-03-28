package format

import (
	"av/avutil"
	"format/aac"
	"format/flv"
	"format/rtmp"
)

func RegisterAll() {
	avutil.DefaultHandlers.Add(rtmp.Handler)
	avutil.DefaultHandlers.Add(flv.Handler)
	avutil.DefaultHandlers.Add(aac.Handler)
}
