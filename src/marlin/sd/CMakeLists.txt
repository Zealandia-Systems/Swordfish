target_sources(${PROJECT_NAME}.elf
	PRIVATE
		cardreader.cpp
		cardreader.h
		Sd2Card_sdio.h
		Sd2Card.cpp
		Sd2Card.h
		SdBaseFile.cpp
		SdBaseFile.h
		SdFatConfig.h
		SdFatStructs.h
		SdFatUtil.cpp
		SdFatUtil.h
		SdFile.cpp
		SdFile.h
		SdInfo.h
		SdVolume.cpp
		SdVolume.h
)
