#ifndef H_CONFIGURATOR
#define H_CONFIGURATOR

#include <fstream>
#include <string>
#include <gl/glew.h>

#include "gld_constants.h"
#include "gld_types.h"
#include "gld_drawing.h"
#include "gld_interface.h"


struct ConfigField {
public:
	unsigned visualizationType;
	Box border;
	ConfigField(int x1, int y1, int x2, int y2);
	void moveScaleHandle(Point point);
	void makeSquare();
	void draw(bool selected);
};

class gldConfigurator {
private:
	PopupMenu popupDefault, popupOnField;
	Point mouseClickPosition;
	bool addingField;
	bool movingField;
	bool resizingField;
	int selectedField;
	std::vector<ConfigField> field;
	bool addField();
	bool addField(int x1, int y1, int x2, int y2);
	void deleteField(int which);
	void deleteAllFields();
	unsigned fieldCount();
	bool selectFieldUnderMouse();
	bool selectFieldHandleUnderMouse();
	void loadPopupMenus();
	virtual void OnMouseDown(int x, int y, mouseButton btn);

public:
	void init();
	void render();
	void mouseDown(mouseButton button);
	void mouseUp(mouseButton button);
	unsigned numberOfField();
	bool saveConfiguration();
	bool loadConfiguration();
};

#endif