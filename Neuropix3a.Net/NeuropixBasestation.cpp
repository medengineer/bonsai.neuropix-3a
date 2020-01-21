#include "stdafx.h"
#include <msclr/marshal_cppstd.h>
#include "NeuropixBasestation.h"

Neuropix3a::Net::NeuropixBasestation::NeuropixBasestation():
	api(new Neuropix_basestation_api())
{
}

Neuropix3a::Net::NeuropixBasestation::!NeuropixBasestation()
{
	delete api;
}

void Neuropix3a::Net::NeuropixBasestation::ThrowExceptionForErrorCode(ErrorCode error, String ^message)
{
	if (error != SUCCESS)
	{
		throw gcnew NeuropixException(message, error);
	}
}

void Neuropix3a::Net::NeuropixBasestation::ThrowExceptionForOpenErrorCode(OpenErrorCode error, String ^message)
{
	if (error != OPEN_SUCCESS)
	{
		throw gcnew NeuropixException(message, error);
	}
}

void Neuropix3a::Net::NeuropixBasestation::ThrowExceptionForConfigAccessErrorCode(ConfigAccessErrorCode error, String ^message)
{
	if (error != CONFIG_SUCCESS)
	{
		throw gcnew NeuropixException(message, error);
	}
}

void Neuropix3a::Net::NeuropixBasestation::ThrowExceptionForEepromErrorCode(EepromErrorCode error, String ^message)
{
	if (error != EEPROM_SUCCESS)
	{
		throw gcnew NeuropixException(message, error);
	}
}

void Neuropix3a::Net::NeuropixBasestation::ThrowExceptionForDigitalControlErrorCode(DigitalControlErrorCode error, String ^message)
{
	if (error != DIGCTRL_SUCCESS)
	{
		throw gcnew NeuropixException(message, error);
	}
}

void Neuropix3a::Net::NeuropixBasestation::ThrowExceptionForReadCsvErrorCode(ReadCsvErrorCode error, String ^message)
{
	if (error != READCSV_SUCCESS)
	{
		throw gcnew NeuropixException(message, error);
	}
}

void Neuropix3a::Net::NeuropixBasestation::ThrowExceptionForBaseConfigErrorCode(BaseConfigErrorCode error, String ^message)
{
	if (error != BASECONFIG_SUCCESS)
	{
		throw gcnew NeuropixException(message, error);
	}
}

void Neuropix3a::Net::NeuropixBasestation::ThrowExceptionForReadErrorCode(ReadErrorCode error, String ^message)
{
	if (error != READ_SUCCESS)
	{
		throw gcnew NeuropixException(message, error);
	}
}

void Neuropix3a::Net::NeuropixBasestation::Open()
{
	Open(0);
}

void Neuropix3a::Net::NeuropixBasestation::Open(Byte headstageSelect)
{
	OpenErrorCode error = api->neuropix_open();
	ThrowExceptionForOpenErrorCode(error, "Unable to open data and configuration link to neuropixel basestation.");
}

void Neuropix3a::Net::NeuropixBasestation::Open(String ^playbackFile)
{
	std::string _playbackFile = msclr::interop::marshal_as<std::string>(playbackFile);
	OpenErrorCode error = api->neuropix_open(_playbackFile);
	ThrowExceptionForOpenErrorCode(error, "Unable to open the specified neuropixel data file.");
}

void Neuropix3a::Net::NeuropixBasestation::Close()
{
	api->neuropix_close();
}

void Neuropix3a::Net::NeuropixBasestation::WriteAllAPGains(GainSetting apGain)
{
	BaseConfigErrorCode error = api->neuropix_writeAllAPGains((int)apGain);
	ThrowExceptionForBaseConfigErrorCode(error, "Unable to write AP gains.");
}

void Neuropix3a::Net::NeuropixBasestation::WriteAllLFPGains(GainSetting lfpGain)
{
	BaseConfigErrorCode error = api->neuropix_writeAllLFPGains((int)lfpGain);
	ThrowExceptionForBaseConfigErrorCode(error, "Unable to write LFP gains.");
}

void Neuropix3a::Net::NeuropixBasestation::ApplyAdcCalibrationFromEeprom()
{
	ErrorCode error = api->neuropix_applyAdcCalibrationFromEeprom();
	ThrowExceptionForErrorCode(error, "Unable to apply ADC calibration.");
}

void Neuropix3a::Net::NeuropixBasestation::ApplyAdcCalibrationFromCsv(String ^comparatorCalibrationFileName, String ^adcOffsetCalibrationFileName, String ^adcSlopeCalibrationFileName)
{
	std::string _comparatorCalibrationFileName = msclr::interop::marshal_as<std::string>(comparatorCalibrationFileName);
	ReadCsvErrorCode csvError = api->neuropix_readComparatorCalibrationFromCsv(_comparatorCalibrationFileName);
	ThrowExceptionForReadCsvErrorCode(csvError, "Unable to read comparator calibration data from the specified file.");

	std::string _adcOffsetCalibrationFileName = msclr::interop::marshal_as<std::string>(adcOffsetCalibrationFileName);
	csvError = api->neuropix_readADCOffsetCalibrationFromCsv(_adcOffsetCalibrationFileName);
	ThrowExceptionForReadCsvErrorCode(csvError, "Unable to read ADC offset calibration data from the specified file.");

	std::string _adcSlopeCalibrationFileName = msclr::interop::marshal_as<std::string>(adcSlopeCalibrationFileName);
	csvError = api->neuropix_readADCSlopeCalibrationFromCsv(_adcSlopeCalibrationFileName);
	ThrowExceptionForReadCsvErrorCode(csvError, "Unable to read ADC slope calibration data from the specified file.");

	std::vector<adcComp> adcCompC;
	std::vector<adcPairCommon> adcPairCommonC;
	ErrorCode error = api->neuropix_getADCCompCalibration(adcCompC);
	ThrowExceptionForErrorCode(error, "Unable to read ADC comparator calibration.");
	error = api->neuropix_getADCPairCommonCalibration(adcPairCommonC);
	ThrowExceptionForErrorCode(error, "Unable to read ADC offset and slope calibration.");

	// Write parameters to probe
	BaseConfigErrorCode configError;
	for (int i = 0; i < 15; i += 2)
	{
		configError = api->neuropix_ADCCalibration(
			i,
			adcCompC[2 * i].compP,
			adcCompC[2 * i].compN,
			adcCompC[2 * i + 2].compP,
			adcCompC[2 * i + 2].compN,
			adcPairCommonC[i].slope,
			adcPairCommonC[i].fine,
			adcPairCommonC[i].coarse,
			adcPairCommonC[i].cfix);
		ThrowExceptionForBaseConfigErrorCode(configError, "Unable to write ADC configuration register on the probe.");
		configError = api->neuropix_ADCCalibration(
			i + 1,
			adcCompC[2 * i + 1].compP,
			adcCompC[2 * i + 1].compN,
			adcCompC[2 * i + 3].compP,
			adcCompC[2 * i + 3].compN,
			adcPairCommonC[i + 1].slope,
			adcPairCommonC[i + 1].fine,
			adcPairCommonC[i + 1].coarse,
			adcPairCommonC[i + 1].cfix);
		ThrowExceptionForBaseConfigErrorCode(configError, "Unable to write ADC configuration register on the probe.");
	}
}

void Neuropix3a::Net::NeuropixBasestation::ApplyGainCalibrationFromEeprom()
{
	ErrorCode error = api->neuropix_applyGainCalibrationFromEeprom();
	ThrowExceptionForErrorCode(error, "Unable to apply Gain calibration.");
}

void Neuropix3a::Net::NeuropixBasestation::ApplyGainCalibrationFromCsv(String ^fileName)
{
	std::string _fileName = msclr::interop::marshal_as<std::string>(fileName);
	ReadCsvErrorCode csvError = api->neuropix_readGainCalibrationFromCsv(_fileName);
	ThrowExceptionForReadCsvErrorCode(csvError, "Unable to read gain calibration data from the specified file.");

	std::vector<unsigned short> gainCorrectionData;
	ErrorCode error = api->neuropix_getGainCorrectionCalibration(gainCorrectionData);
	ThrowExceptionForErrorCode(error, "Unable to read gain calibration.");

	// Resize according to probe type
	unsigned int option = api->neuropix_getOption();
	if (option < 2)
		gainCorrectionData.resize(384);
	else if (option == 2)
		gainCorrectionData.resize(960);
	else if (option == 3) 
		gainCorrectionData.resize(966);

	//Write to basestation FPGA
	ConfigAccessErrorCode configError = api->neuropix_gainCorrection(gainCorrectionData);
	ThrowExceptionForConfigAccessErrorCode(configError, "Unable to write gain calibration to FPGA registers.");
}

void Neuropix3a::Net::NeuropixBasestation::ConfigureDeserializer()
{
	ErrorCode error = api->neuropix_configureDeserializer();
	ThrowExceptionForErrorCode(error, "Unable to configure deserializer.");
}

void Neuropix3a::Net::NeuropixBasestation::ConfigureSerializer()
{
	ErrorCode error = api->neuropix_configureSerializer();
	ThrowExceptionForErrorCode(error, "Unable to configure serializer.");
}

Neuropix3a::Net::VersionNumber Neuropix3a::Net::NeuropixBasestation::GetHardwareVersion()
{
	::VersionNumber version;
	ErrorCode error = api->neuropix_getHardwareVersion(&version);
	ThrowExceptionForErrorCode(error, "No config link connection.");
	return VersionNumber(version.major, version.minor);
}

Neuropix3a::Net::VersionNumber Neuropix3a::Net::NeuropixBasestation::GetAPIVersion()
{
	::VersionNumber version = api->neuropix_getAPIVersion();
	return VersionNumber(version.major, version.minor);
}

Neuropix3a::Net::VersionNumber Neuropix3a::Net::NeuropixBasestation::GetBSVersion()
{
	unsigned char major, minor;
	ConfigAccessErrorCode error = api->neuropix_getBSVersion(major);
	ThrowExceptionForConfigAccessErrorCode(error, "No existing config link.");
	error = api->neuropix_getBSRevision(minor);
	ThrowExceptionForConfigAccessErrorCode(error, "No existing config link.");
	return VersionNumber(major, minor);
}

Neuropix3a::Net::AsicID Neuropix3a::Net::NeuropixBasestation::ReadID()
{
	::AsicID id;
	EepromErrorCode error = api->neuropix_readId(id);
	ThrowExceptionForEepromErrorCode(error, "Error reading probe ID from EEPROM.");
	return AsicID(id.serialNumber, id.probeType);
}

void Neuropix3a::Net::NeuropixBasestation::WriteID(Neuropix3a::Net::AsicID id)
{
	::AsicID _id;
	_id.probeType = id.ProbeType;
	_id.serialNumber = id.SerialNumber;
	EepromErrorCode error = api->neuropix_writeId(_id);
	ThrowExceptionForEepromErrorCode(error, "Error writing probe ID to EEPROM.");
}

unsigned char Neuropix3a::Net::NeuropixBasestation::GetOption()
{
	return api->neuropix_getOption();
}

void Neuropix3a::Net::NeuropixBasestation::StartLog()
{
	api->neuropix_startLog();
}

void Neuropix3a::Net::NeuropixBasestation::LedOff(bool ledOff)
{
	DigitalControlErrorCode error = api->neuropix_ledOff(ledOff);
	ThrowExceptionForDigitalControlErrorCode(error, "Error setting headstage LED state.");
}

void Neuropix3a::Net::NeuropixBasestation::SetFilter(FilterBandwidth filter)
{
	BaseConfigErrorCode error = api->neuropix_setFilter((int)filter);
	ThrowExceptionForBaseConfigErrorCode(error, "Unable to set filter bandwidth.");
}

void Neuropix3a::Net::NeuropixBasestation::SetNrst(bool nrst)
{
	DigitalControlErrorCode error = api->neuropix_nrst(nrst);
	ThrowExceptionForDigitalControlErrorCode(error, "Unable to set nrst register.");
}

void Neuropix3a::Net::NeuropixBasestation::NeuralStart()
{
	ConfigAccessErrorCode error = api->neuropix_setNeuralStart();
	ThrowExceptionForConfigAccessErrorCode(error, "Unable to set neural start trigger.");
}

void Neuropix3a::Net::NeuropixBasestation::ResetDatapath()
{
	ErrorCode error = api->neuropix_resetDatapath();
	ThrowExceptionForErrorCode(error, "Error resetting datapath.");
}

bool Neuropix3a::Net::NeuropixBasestation::ReadElectrodeData(ElectrodePacket ^packet)
{
	ReadErrorCode error = api->neuropix_readElectrodeData(*(packet->packet));
	if (error == DATA_BUFFER_EMPTY) return false;
	ThrowExceptionForReadErrorCode(error, "Unable to read electrode data.");
	return true;
}

void Neuropix3a::Net::NeuropixBasestation::StartRecording(String ^fileName)
{
	std::string _fileName = msclr::interop::marshal_as<std::string>(fileName);
	ErrorCode error = api->neuropix_startRecording(_fileName);
	ThrowExceptionForErrorCode(error, "No tcp/ip data connection.");
}

void Neuropix3a::Net::NeuropixBasestation::StopRecording()
{
	ErrorCode error = api->neuropix_stopRecording();
	ThrowExceptionForErrorCode(error, "No tcp/ip data connection.");
}