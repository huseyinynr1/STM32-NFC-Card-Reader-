#include "firmware_update_helper.h"

void Firmware_Check_Response_Json_Convert_to_Object(char* arr, http_check_firmware_typedef* response_object)
{
	char* p;

	p = strstr(arr, "\"Success\":");

	if(p != NULL)
	{
		p += strlen("\"Success\":");
		if(strncmp(p, "true", 4) == 0)
		{
			response_object->success = true;
			response_object->isActive = true;
		}

		else
		{
			response_object->success = false;
			response_object->isActive = false;
			return;
		}
	}

	p = strstr(arr, "\"FirmwareId\":");
	if(p != NULL)
	{
		p += strlen("\"FirmwareId\":");
		response_object->firmware_id = atoi(p);
	}

	p = strstr(arr, "\"Version\":");
	if(p != NULL)
	{
		p += strlen("\"Version\":\"");
		char* end = strchr(p, '"');
		if(end != NULL && end > p)
		{
			char version_text[12] = {0};
			size_t len = end - p;

			if(len < sizeof(version_text))
			{
				memcpy(version_text, p , len);
				version_text[len] = '\0';

				int major = 0;
				int minor = 0;
				int patch = 0;
				if(sscanf(version_text, "%d.%d.%d", &major, &minor, &patch) == 3)
				{
					response_object->firmware_version = Helper_Convert_Firmware_Version((uint8_t)major, (uint8_t)minor, (uint8_t)patch);
				}
			}
		}
	}

	p = strstr(arr, "\"FileSizeBytes\":");
	if(p != NULL)
	{
		p += strlen("\"FileSizeBytes\":");
		response_object->firmware_size = atoi(p);
	}

	p = strstr(arr, "\"Crc32Hex\":");
	if(p != NULL)
	{
		p += strlen("\"Crc32Hex\":\"");
		char* end = strchr(p, '"');
		if(end != NULL && end > p)
		{
			char crc_text[8] = {0};
			uint32_t len = end - p;

			if(len == 8)
			{
				memcpy(crc_text, p, len);
				response_object->firmware_crc32 = (uint32_t)strtoul(crc_text, NULL, 16);
			}
		}
	}
}

// Firmware karta başarıyla yüklendiği cevabı Json olarak hazırlamak.
void Firmware_Update_IsActive_Convert_Json(char* arr, uint8_t firmware_id, bool is_active)
{
	sprintf(arr, "{\"FirmwareId\":%u,\"IsActive\":%s}", firmware_id, is_active ? "true" : "false");
}

void Firmware_Update_IsActive_Response_Json_Convert(char* arr, bool* success)
{
	char* p;

	p = strstr(arr, "\"Success\":");

	if(p != NULL)
	{
		p += strlen("\"Success\":");
		if(strncmp(p, "true", 4) == 0)
		{
			*success = true;
		}

		else
		{
			*success = false;
			return;
		}
	}
}

// Yeni firmware dosya parça değerini almak GET isteği ile almak için gerekli URL'i oluşturan fonksiyon
void Firmware_Build_Chunk_URL(char* out_url,
                              size_t out_url_size,
                              const char* base_url,
                              uint32_t firmware_id,
                              uint32_t offset,
                              uint16_t chunk_size)
{
	/*
	 *  Dışarıdan gelen array'e ana endpoint ve sorgu için gerekli firmware id,
	 *  şuanki parça değeri ve alınacak parça değerini birleştirerek array'e kaydet.
	 *  GET ile sorgu yapılacak URL'i oluştur.
	 */
    snprintf(out_url,
             out_url_size,
             "%s?firmwareId=%lu&offset=%lu&size=%lu",
             base_url,
             firmware_id,
             offset,
             chunk_size);
}


uint32_t Helper_Convert_Firmware_Version(uint8_t major, uint8_t minor, uint8_t patch)
{
    return ((uint32_t)major << 16) |
           ((uint32_t)minor << 8)  |
           ((uint32_t)patch);
}
