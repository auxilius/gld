#include <iomanip>
#include "gld_types.h"


inputManager input;
Box canvas;
IOBox form;
HWND windowHandle;
HDC deviceContext;
ConfigurationLoader configLoader;
ProfileSwitcher profileSwitcher;
std::string SAVE_PATH = "";

GLcolor::GLcolor() {
	set(0.0f, 0.0f, 0.0f);
};
GLcolor::GLcolor(GLfloat r, GLfloat g, GLfloat b) {
	set(r, g, b);
};
void GLcolor::set(GLfloat r, GLfloat g, GLfloat b) {
	R = r;
	G = g;
	B = b;
};


void Point::set(int sx, int sy) {
	x = sx; y = sy;
};
Point point(int x, int y) {
	Point result;
	result.set(x, y);
	return result;
};

double distance(float x1, float y1, float x2, float y2) {
	return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
};
double pointDistance(Point p1, Point p2) {
	return sqrt(pow((long double)(p1.x - p2.x), 2) + pow((long double)(p1.y - p2.y), 2));
};

Point Box::getPosition() {
	Point result;
	result.set(left, top);
	return result;
};
Point Box::getCenterPosition() {
	Point result;
	result.set((left + right) / 2, (top + bottom) / 2);
	return result;
};
void Box::set(int x1, int y1, int x2, int y2) 
{
	if (x1 > x2)
		std::swap(x1, x2);
	if (y1 > y2)
		std::swap(y1, y2);
	top = y1; left = x1; bottom = y2; right = x2;
};
void Box::set(int x, int y, int size)
{
	left = x;
	top = y; 
	right = x+size;
	bottom = y+size;
};
void Box::set(Box border) {
	set(border.left, border.top, border.right, border.bottom);
};
bool Box::contains(int x, int y) 
{
	return (x > left && x <= right && y > top && y <= bottom);
};
bool Box::contains(Point position)
{
	return (position.x >= left && position.x <= right && position.y >= top && position.y <= bottom);
};
void Box::move(int vx, int vy) 
{
	top += vy; bottom += vy; left += vx; right += vx;
};
void Box::moveTo(int x, int y) 
{
	int width = right - left;
	int height = bottom - top;
	top = y; bottom = y + height;
	left = x; right = x + width;
};
int Box::getWidth() 
{
	return right - left;
};
int Box::getHeight() 
{
	return bottom - top;
};
void Box::setWidth(int newWidth)
{
	if (newWidth < 0) 
		return;
	right = left + newWidth;
};
void Box::setHeight(int newHeight)
{
	if (newHeight < 0)
		return;
	bottom = top + newHeight;
};
void Box::setSize(int newWidth, int newHeight)
{
	setWidth(newWidth);
	setHeight(newHeight);
};


void IOBox::save(std::ofstream & stream) {
	stream << left << " " << top << " " << right << " " << bottom << " ";

};
void IOBox::load(std::ifstream & stream) {
	stream >> left >> top >> right >> bottom;
};


ConfigFieldRecord::ConfigFieldRecord() {
	type = 0;
};
void ConfigFieldRecord::save(std::ofstream & stream) {
	border.save(stream);
	stream << type << " ";
	stream << param_i.size() << " ";
	for (unsigned i = 0; i < param_i.size(); i++)
		stream << param_i[i] << " ";
	stream << param_d.size() << " ";
	for (unsigned i = 0; i < param_d.size(); i++)
		stream << std::fixed << std::setprecision(5) << param_d[i] << " ";
	if (param_str.empty()) stream << "-";
	else stream << param_str.c_str();
	stream << std::endl;
};
void ConfigFieldRecord::load(std::ifstream & stream) {
	border.load(stream);
	stream >> type;
	param_i.clear();
	unsigned paramCount = 0;
	stream >> paramCount;
	int newParam_i;
	for (unsigned i = 0; i < paramCount; i++) {
		stream >> newParam_i;
		param_i.push_back(newParam_i);
	}
	param_d.clear();
	stream >> paramCount;
	double newParam_d;
	for (unsigned i = 0; i < paramCount; i++) {
		stream >> newParam_d;
		param_d.push_back(newParam_d);
	}
	std::getline(stream, param_str);
	param_str.erase(param_str.begin());
};



#pragma region ConfigurationLoader

ConfigurationLoader::ConfigurationLoader() {
	openedFile = NULL;
};
bool ConfigurationLoader::fieldsBlank() {
	for (unsigned i = 0; i < field.size(); i++)
		if (field[i].type != FIELD_TYPE_NONE)
			return false;
	return true;
};
bool ConfigurationLoader::save() {
	if (openedFile == NULL)
		return false;
	std::ofstream stream ( openedFile );
	if (stream.fail()) {
		std::cout << "ERROR saving config file " << openedFile << std::endl;
		MessageBox(0, L"Cannot create configuration file, location may be write protected.", L"Saving configuration failed", MB_OK | MB_ICONWARNING);
		return false;
	}
	stream << CONFIG_FILE_VERSION << std::endl;
	form.save(stream);
	stream << std::endl;
	stream << field.size() << std::endl;
	for (unsigned i = 0; i < field.size(); i++)
		field[i].save(stream);
	stream.close();
	return true;
};
bool ConfigurationLoader::open(char * fileName) {
	field.clear();
	if (!fileExists(fileName)) {
		fileCreate(fileName);
		openedFile = fileName;
		return true;
	}
	std::ifstream inStream ( fileName );
	if (inStream.fail()) {
		MessageBox(0, L"Error opening config file.", L"GLD - loading error", MB_OK | MB_ICONWARNING);
		return false;
	}
	std::string version;
	getline(inStream, version);
	if (version != CONFIG_FILE_VERSION && version != "") {
		MessageBox(0, L"Version of configuration file is out of date. Configuration will not load.", L"GLD - loading error", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// READ - SKIPPING WIDTH, HEIGHT, X, Y
	form.load(inStream);
	SetWindowPos(windowHandle, 0, form.left, form.top, form.width, form.height, SWP_NOZORDER|SWP_NOACTIVATE);
	// READ - NUMBER OF FIELDS
	unsigned count = 0;
	inStream >> count;
	// READ - FIELDS
	ConfigFieldRecord newField;
	for (unsigned i = 0; i < count; i++) {
		newField.load(inStream);
		field.push_back(newField);
	}
	inStream.close();
	openedFile = fileName;
	return true;
};
bool ConfigurationLoader::fieldSetType(int f, int t) {
	if (f < (int)field.size() && f >= 0) {
		if (field[f].type == t)
			return false;
		else {
			field[f].param_i.clear();
			field[f].param_d.clear();
			field[f].param_str = "";
			field[f].type = t;
			return true;
		}
	}
	return false;
};

/* void ConfigurationLoader::debugOut() {
	std::cout << "-------------------------------------" << std::endl;
	std::cout << "Configuration:" << std::endl;
	for (unsigned i = 0; i < field.size(); i++) {
		ConfigFieldRecord * f = &field[i];
		std::cout << "Field " << i << " - " << f->border.left << " " << f->border.top << " " << f->border.right << " " << f->border.bottom << std::endl;
		std::cout << "  type: " << f->type << std::endl;
		std::cout << "  param_i: ";
		for (unsigned x = 0; x < f->param_i.size(); x++)
			std::cout << f->param_i[x] << " ";
		std::cout << std::endl;
		std::cout << "  param_d: ";
		for (unsigned x = 0; x < f->param_d.size(); x++)
			std::cout << f->param_d[x] << " ";
		std::cout << std::endl;
		std::cout << "  param_str: '" << f->param_str << "'" << std::endl;
	}
};
*/

#pragma endregion



#pragma region ProfileSwitcher

void ProfileSwitcher::init() {
	profiles.clear();
	char* profDefault = PROFILE_CAPTION_DEFAULT;
	profiles.push_back( profDefault );
	this->load();
	actualProfile = 0;
	configLoader.open( getConfigFileName() );
};

void ProfileSwitcher::selectProfile(int selected) {
	if (selected < 0 || selected >= (int)profiles.size() )
		return;
	if (actualProfile != selected) {
		actualProfile = selected;
		configLoader.open( getConfigFileName() );
	}
};

bool ProfileSwitcher::addProfile(std::string name) {
	if (std::find(profiles.begin(), profiles.end(), name) != profiles.end() )
		return false;
	profiles.push_back(name);
	selectProfile( profiles.size()-1 );
	this->save();
	return true;
};

void ProfileSwitcher::deleteProfile(int profile_id) {
	if ( profile_id<=0 || profile_id>=(int)profiles.size())
		return;
	char * configFile = getConfigFileName(profile_id);
	profiles.erase(profiles.begin()+profile_id);
	remove(configFile);
	if (profile_id == actualProfile)
		selectProfile(0);
	if (profile_id < actualProfile)
		actualProfile--;
	this->save();
};

void ProfileSwitcher::renameProfile(int profile_id, std::string name) {
	if ( profile_id<=0 || profile_id>=(int)profiles.size())
		return;
	char * oldConfigFile = getConfigFileName(profile_id);
	profiles[profile_id] = name;
	rename(oldConfigFile, getConfigFileName(profile_id) );
	if (profile_id == actualProfile)
		configLoader.open( getConfigFileName() );
	this->save();
};

bool ProfileSwitcher::load() {
	char * fName = pathToFile(FILE_PROFILES);
	if (!fileExists(fName)) {
		fileCreate( fName );
		return false;
	}
	std::ifstream inStream;
	inStream.open( fName );
	if (inStream.fail()) {
		MessageBox(0, L"Error opening file containing profiles.", 
			L"GLD - loading error", MB_OK | MB_ICONWARNING);
		return false;
	}
	std::string cLine;
	while (! inStream.eof() ) {
		getline(inStream, cLine);
		cLine = trim(cLine);
		if ( (cLine == "") || (cLine[0] == '#') )
			continue;
		profiles.push_back( cLine );
	}
	inStream.close();
	return true;
};

void ProfileSwitcher::save() {
	std::ofstream outStream;
	outStream.open(pathToFile(FILE_PROFILES));
	if (outStream.fail())
		MessageBox(0, L"Cannot save profiles into file.", L"GLD - save error", MB_OK | MB_ICONWARNING);
	for (int i = 1; i < (int)profiles.size(); i++) { 
		outStream << profiles[i] << std::endl;
	}
	outStream.close();
};

char* ProfileSwitcher::getProfileName(int profile_id) {
	if (profile_id >= 0 && profile_id < (int)profiles.size() )
		return stringToChar( profiles[profile_id] );
	else if (profile_id == -1)
		return stringToChar( profiles[actualProfile] );
	else
		return "error_profile_id";
};

char* ProfileSwitcher::getConfigFileName(int profile_id) {
	std::string fname  = (std::string)getProfileName(profile_id) + FILE_EXT_CONFIG;
	if ( (profile_id == -1 && actualProfile == 0) || profile_id == 0)
		fname = FILE_PROFILE_DEFAULT;
	return pathToFile(stringToChar(fname));
};

unsigned ProfileSwitcher::count() {
	return profiles.size();
};

#pragma endregion



#pragma region Utils

bool fileExists(char* fileName) {
	std::ifstream infile(fileName);
	bool result = infile.good();
	infile.close();
	return result;
};

bool dirExists(const std::string& dirName_in) {
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false; 
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true; 
	return false;
};

void fileCreate(char* fileName) {
	std::ofstream outfile ( fileName );
	outfile.close();
};

char * pathToFile(char* fname) {
	std::string path = SAVE_PATH + fname;
	char * result = stringToChar(path);
	return result;
};

char * stringToChar(std::string str) {
	char * writable = new char[str.size() + 1];
	strcpy_s(writable, str.size() + 1, str.c_str());
	return writable;
};

std::string trim(std::string str) {
	if (str.length() == 0)
		return str;
	int s = 0;
	int e = str.length()-1;
	while ( (str[s] == ' ') && (s < e) ) 
		s++;
	while ( (str[e] == ' ') && (s < e) ) 
		e--;
	return str.substr(s, e-s+1);
};

#pragma endregion