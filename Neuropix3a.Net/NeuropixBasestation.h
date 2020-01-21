#pragma once

using namespace System;
using namespace System::Runtime::Serialization;
#include "Neuropix_basestation_api.h"
#include "ElectrodePacket.h"

namespace Neuropix3a
{
	namespace Net
	{
		public value class AsicID {
		public:
			unsigned int SerialNumber;
			unsigned char ProbeType;

			AsicID(unsigned int serialNumber, char probeType):
				SerialNumber(serialNumber),
				ProbeType(probeType)
			{
			}
		};

		public value class VersionNumber
		{
		public:
			unsigned short Major;
			unsigned short Minor;

			VersionNumber(unsigned short major, unsigned short minor):
				Major(major),
				Minor(minor)
			{
			}
		};

		public enum class AsicMode
		{
			Configuration = ASIC_CONFIGURATION,
			Calibration = ASIC_CALIBRATION,
			Impedance = ASIC_IMPEDANCE,
			Recording = ASIC_RECORDING
		};

		public enum class FilterBandwidth
		{
			HighPass300Hz = 0,
			HighPass500Hz = 1,
			HighPass1kHz = 3
		};

		public enum class GainSetting
		{
			Gain50 = 0,
			Gain125 = 1,
			Gain250 = 2,
			Gain500 = 3,
			Gain1000 = 4,
			Gain1500 = 5,
			Gain2000 = 6,
			Gain2500 = 7,
		};

		[SerializableAttribute]
		public ref class NeuropixException : public Exception
		{
		private:
			int errorCode;
		public:
			NeuropixException() { };
			NeuropixException(String ^message): Exception(message) { }
			NeuropixException(String ^message, Exception ^innerException) : Exception(message, innerException) { }
			NeuropixException(String ^message, int errorCode): Exception(message), errorCode(errorCode) { }
			NeuropixException(String ^message, int errorCode, Exception ^innerException) : Exception(message, innerException), errorCode(errorCode) { }

			property int ErrorCode
			{
				int get() { return errorCode; }
			}

			void GetObjectData(SerializationInfo ^info, StreamingContext context) override
			{
				if (info == nullptr)
				{
					throw gcnew ArgumentNullException("info");
				}

				info->AddValue("ErrorCode", errorCode);
				Exception::GetObjectData(info, context);
			}
		protected:
			NeuropixException(SerializationInfo ^info, StreamingContext context):
				Exception(info, context),
				errorCode((int)info->GetValue("ErrorCode", Int32::typeid))
			{
			}
		};

		public ref class NeuropixBasestation
		{
		private:
			Neuropix_basestation_api *api;
			static void ThrowExceptionForErrorCode(ErrorCode error, String ^message);
			static void ThrowExceptionForOpenErrorCode(OpenErrorCode error, String ^message);
			static void ThrowExceptionForConfigAccessErrorCode(ConfigAccessErrorCode error, String ^message);
			static void ThrowExceptionForEepromErrorCode(EepromErrorCode error, String ^message);
			static void ThrowExceptionForDigitalControlErrorCode(DigitalControlErrorCode error, String ^message);
			static void ThrowExceptionForReadCsvErrorCode(ReadCsvErrorCode error, String ^message);
			static void ThrowExceptionForBaseConfigErrorCode(BaseConfigErrorCode error, String ^message);
			static void ThrowExceptionForReadErrorCode(ReadErrorCode error, String ^message);
			~NeuropixBasestation() { this->!NeuropixBasestation(); }
			!NeuropixBasestation();
		public:
			NeuropixBasestation();

			void Open();
			void Open(Byte headstageSelect);
			void Open(String ^playbackFile);
			void Close();

			VersionNumber GetHardwareVersion();
			VersionNumber GetAPIVersion();
			VersionNumber GetBSVersion();

			AsicID ReadID();
			void WriteID(AsicID id);
			unsigned char GetOption();

			void StartLog();
			void LedOff(bool ledOff);

			void WriteAllAPGains(GainSetting apGain);
			void WriteAllLFPGains(GainSetting lfpGain);

			void ApplyAdcCalibrationFromEeprom();
			void ApplyAdcCalibrationFromCsv(String ^comparatorCalibrationFileName, String ^adcOffsetCalibrationFileName, String ^adcSlopeCalibrationFileName);

			void ApplyGainCalibrationFromEeprom();
			void ApplyGainCalibrationFromCsv(String ^gainCalibrationFileName);
			void ConfigureDeserializer();
			void ConfigureSerializer();

			void SetFilter(FilterBandwidth filter);
			void SetNrst(bool nrst);

			void NeuralStart();
			void ResetDatapath();
			bool ReadElectrodeData(ElectrodePacket ^packet);
			void StartRecording(String ^fileName);
			void StopRecording();

			property bool IsConnected {
				bool get()
				{
					return api->neuropix_isConnected();
				}
			}

			property float FifoFilling {
				float get()
				{
					return api->neuropix_fifoFilling();
				}
			}

			property float ScaleFactorToVoltage {
				float get()
				{
					return api->neuropix_getScaleFactorToVoltage();
				}
			}

			property AsicMode Mode {
				AsicMode get()
				{
					unsigned char mode;
					DigitalControlErrorCode error = api->neuropix_readMode(mode);
					ThrowExceptionForDigitalControlErrorCode(error, "Unable to read ASIC mode.");
					return (AsicMode)mode;
				}
				void set(AsicMode value)
				{
					DigitalControlErrorCode error = api->neuropix_mode((unsigned char)value);
					ThrowExceptionForDigitalControlErrorCode(error, "Unable to set ASIC mode.");
				}
			}

			property bool DataMode {
				bool get() { return api->neuropix_getDatamode(); }
				void set(bool value)
				{
					ErrorCode error = api->neuropix_datamode(value);
					ThrowExceptionForErrorCode(error, "Unable to set electrode data mode.");
				}
			}

			property bool TriggerMode {
				bool get()
				{
					bool triggerMode;
					ConfigAccessErrorCode error = api->neuropix_getTriggerMode(triggerMode);
					ThrowExceptionForConfigAccessErrorCode(error, "Unable to read trigger mode.");
					return triggerMode;
				}
				void set(bool value)
				{
					ConfigAccessErrorCode error = api->neuropix_triggerMode(value);
					ThrowExceptionForConfigAccessErrorCode(error, "Unable to set trigger mode.");
				}
			}
		};
	}
}