#include "igc.h"
#include "logger.h"
#include "../../drivers/storage/storage.h"

void kml_writeline(char * line)
{
	uint8_t l = strlen(line);
	uint16_t wl;

//	DEBUG("KML:%s\n", line);

	strcpy_P(line + l, PSTR("\r\n"));
	l += 2;

	assert(f_write(log_fil, line, l, &wl) == FR_OK);
	assert(wl == l);
	assert(f_sync(log_fil) == FR_OK);
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

	datetime_from_epoch(fc.gps_data.utc_time, &sec, &min, &hour, &day, &wday, &month, &year);

	sprintf_P(filename, PSTR("/%s/%02d-%02d%02d.KML"), path, logger_flight_number, hour, min);
	DEBUG("KML filename %s\n", filename);

	uint8_t res = f_open(log_fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
	assert(res == FR_OK);

	//cannot create file
	if (res != FR_OK)
		return false;

	//header
	GetID_str(id);
	sprintf_P(line, PSTR("<!-- Generated by SkyDrop vario (www.skybean.eu) -->"));
	kml_writeline(line);
	sprintf_P(line, PSTR("<!--  DATE: %02d.%02d.%04d -->"), day, month, year);
	kml_writeline(line);
	sprintf_P(line, PSTR("<!--  TIME: %02d:%02d -->"), hour, min);
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

	return true;
}

void kml_step()
{
	if (fc.gps_data.valid)
	{
		char line[79];
		sprintf_P(line, PSTR("%0.6f,%0.6f,%0.0f"), fc.gps_data.longtitude, fc.gps_data.latitude, fc.altitude1);
		kml_writeline(line);
	}
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

	assert(f_close(log_fil) == FR_OK);
}
