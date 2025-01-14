/**
 * Copyright 2008 Raven
 */

#include "RMusicPlayerPrivate.h"

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * static constructor (builds configuration menu)
 */
void URMusicPlayerConfig::StaticConstructor()
{
	guard(URMusicPlayerConfig::StaticConstructor);
	//Sound driver configuration
	FMOD::System     *local_system;
	FMOD_RESULT result;
	result = FMOD::System_Create(&local_system);

	int i, numdrivers;	
	UEnum* SoundDrivers = new( GetClass(), TEXT("SoundDriver") )UEnum( NULL );
    
	result = local_system->getNumDrivers(&numdrivers);

	for (i=0; i < numdrivers; i++)
	{
		char StringA[256];
		FMOD_GUID Guid;
		INT SampleRate;
		FMOD_SPEAKERMODE SpeakerMode;
		INT Channels;
		result = local_system->getDriverInfo(i, StringA, 256, &Guid, &SampleRate, &SpeakerMode, &Channels);
		new(SoundDrivers->Names) FName(ANSI_TO_TCHAR(StringA));
	}       

	new(GetClass(), TEXT("SoundDriver"), RF_Public)UStrProperty (CPP_PROPERTY( DriverName ), TEXT("MusicPlayer"), CPF_Config );

	UEnum* SoundOutput = new( GetClass(), TEXT("Output") )UEnum( NULL );    
	new( SoundOutput->Names ) FName( TEXT("Default") ) ;
	new( SoundOutput->Names ) FName( TEXT("WASAPI") ) ;
	new( SoundOutput->Names ) FName( TEXT("ASIO") ) ;
	new(GetClass(), TEXT("Output"), RF_Public)UByteProperty(CPP_PROPERTY( Output ), TEXT("MusicPlayer"), CPF_Config, SoundOutput );
	
	unguard;
}
/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * returns selected driver
 */
int URMusicPlayerConfig::GetDriverNum()
{
	guard(URMusicPlayerConfig::GetDriverNum);
	//Sound driver configuration
	FMOD::System     *local_system;
	FMOD_RESULT result;
	result = FMOD::System_Create(&local_system);

	int i, numdrivers;
	int result_driver = (int)-1;
	FString CrrentDriverF;

	result = local_system->getNumDrivers(&numdrivers);

	for (i=0; i < numdrivers; i++)
	{
		char StringA[256];
		FMOD_GUID Guid;
		INT SampleRate;
		FMOD_SPEAKERMODE SpeakerMode;
		INT Channels;
		result = local_system->getDriverInfo(i, StringA, 256, &Guid, &SampleRate, &SpeakerMode, &Channels);
		CrrentDriverF = (ANSI_TO_TCHAR(StringA));
		
		if(CrrentDriverF == Cast<URMusicPlayerConfig>(URMusicPlayerConfig::StaticClass()->GetDefaultObject() )->DriverName ) 
		{
			result_driver=i;
			break;
		}
	}

	return result_driver;
	unguard;
}

IMPLEMENT_CLASS(URMusicPlayerConfig);