#include "igc.h"
#include "logger.h"
#include "../../drivers/storage/storage.h"

void kml_writeline(char * line)
{
	uint8_t l = strlen(line);
	unsigned int wl;

//	DEBUG("KML:%s\n", line);

	strcpy_P(line + l, PSTR("\r\n"));
	l += 2;

	assert(f_write(&log_file, line, l, &wl) == FR_OK);
	assert(wl == l);

#ifndef STM32
	//this is very slowly on spi flash
	assert(f_sync(&log_file) == FR_OK);
#endif
}

bool kml_start(char * path)
{
	char filename[128];

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;

	char line[79];
	char id[32];

#ifdef GPS_SUPPORT
	datetime_from_epoch(time_get_utc(), &sec, &min, &hour, &day, &wday, &month, &year);
#endif

	sprintf_P(filename, PSTR("/%s/%02d-%02d%02d.KML"), path, logger_flight_number, hour, min);
	DEBUG("KML filename %s\n", filename);

	uint8_t res = f_open(&log_file, filename, FA_WRITE | FA_CREATE_ALWAYS);
	assert(res == FR_OK);

	//cannot create file
	if (res != FR_OK)
		return false;

	//header
	GetID_str(id);
	sprintf_P(line, PSTR("<!-- Generated by SkyDrop vario (www.skybean.eu) -->"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<!--  Date: %02d.%02d.%04d -->"), day, month, year);
	kml_writeline(line);
	sprintf_P(line, PSTR("<!--  Time: %02d:%02d -->"), hour, min);
	kml_writeline(line);
	sprintf_P(line, PSTR("<!--  Pilot: %s -->"), config.logger.pilot);
	kml_writeline(line);
	sprintf_P(line, PSTR("<!--  Glider type: %s -->"), config.logger.glider_type);
	kml_writeline(line);
	sprintf_P(line, PSTR("<!--  Glider ID: %s -->"), config.logger.glider_id);
	kml_writeline(line);
	sprintf_P(line, PSTR("<!--  S/N: %s -->"), id);
	kml_writeline(line);
	sprintf_P(line, PSTR("<!--  HW: drop_%d -->"), (hw_revision == HW_REW_1504) ? 1504 : 1506);
	kml_writeline(line);
	sprintf_P(line, PSTR("<!--  SW: build %d -->"), BUILD_NUMBER);
	kml_writeline(line);

	//body
	sprintf_P(line, PSTR("<kml xmlns=\"http://earth.google.com/kml/2.0\">"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<Document>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<name>Flight log from %02d.%02d.%04d @ %02d:%02d</name>"), day, month, year, hour, min);
	kml_writeline(line);
	sprintf_P(line, PSTR("<Placemark>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<name>Flight</name>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<visibility>1</visibility>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<open>1</open>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<Style>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<LineStyle><color>ff00ffff</color></LineStyle>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<PolyStyle><color>7f0000ff</color></PolyStyle>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("</Style>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<LineString>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<altitudeMode>absolute</altitudeMode>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<extrude>1</extrude>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<coordinates>"));
	kml_writeline(line);

	return (fc.gps_data.valid) ? LOGGER_ACTIVE : LOGGER_WAIT_FOR_GPS;
}

void kml_step()
{
#ifdef GPS_SUPPORT
	if (fc.gps_data.valid)
	{
		char line[79];
		char tmp1[16];
		char tmp2[16];

		fc.logger_state = LOGGER_ACTIVE;

		sprintf_P(tmp1, PSTR(" %+011ld"), fc.gps_data.longtitude);
		memcpy((void *)tmp1, (void *)(tmp1 + 1), 4);
		tmp1[4] = '.';

		sprintf_P(tmp2, PSTR(" %+010ld"), fc.gps_data.latitude);
		memcpy((void *)tmp2, (void *)(tmp2 + 1), 3);
		tmp2[3] = '.';

		sprintf_P(line, PSTR("%s,%s,%0.0f"), tmp1, tmp2, fc.altitude1);
		kml_writeline(line);
	}
	else
#endif
		fc.logger_state = LOGGER_WAIT_FOR_GPS;
}

void kml_comment(char * text)
{
	char line[79];

	sprintf_P(line, PSTR("<!-- %s -->"), text);
	kml_writeline(line);
}


void kml_stop()
{
	char line[79];

	sprintf_P(line, PSTR("</coordinates>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("</LineString>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("</Placemark>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("</Document>"));
	kml_writeline(line);
	sprintf_P(line, PSTR("</kml>"));
	kml_writeline(line);

	assert(f_close(&log_file) == FR_OK);
}
