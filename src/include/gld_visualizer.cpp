#include "gld_visualizer.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <gl\GLU.h>
#include "gld_types.h"

gldRenderer * globalRenderer;

void setColorByValue(float value) {
	if (value >= 0.0f && value < 0.2f) {
		value = value / 0.2f;
		glColor3f(0.0f, value, 1.0f);
	}
	else if (value >= 0.2f && value < 0.4f) {
		value = (value - 0.2f) / 0.2f;
		glColor3f(0.0f, 1.0f, 1.0f - value);
	}
	else if (value >= 0.4f && value < 0.6f) {
		value = (value - 0.4f) / 0.2f;
		glColor3f(value, 1.0f, 0.0f);
	}
	else if (value >= 0.6f && value < 0.8f) {
		value = (value - 0.6f) / 0.2f;
		glColor3f(1.0f, 1.0f - value, 0.0f);
	}
	else if (value >= 0.8f && value <= 1.0f) {
		value = (value - 0.8f) / 0.2f;
		glColor3f(1.0f, 0.0f, value);
	}
	else if (value > 1.0f)
		glColor3f(1.0f, 1.0f, 1.0f);
	else if (value < 0.0f)
		glColor3f(0.0f, 0.0f, 0.0f);
};

void gldRenderer::init() {
	globalRenderer = this;
};
bool gldRenderer::selectModelFieldUnderMouse() {
	int topmost = -1;
	for (unsigned i = 0; i < modelField.size(); i++) {
		if (modelField[i].border.contains(input.mouse)) {
			if (topmost == -1 || modelField[topmost].layer < modelField[i].layer)
				topmost = i;
		}
	}
	selectedModelField = topmost;
	return (topmost != -1);
};
bool gldRenderer::selectTextureFieldUnderMouse() {
	int topmost = -1;
	for (unsigned i = 0; i < textureField.size(); i++) {
		if (textureField[i].border.contains(input.mouse))
			if (topmost == -1 || textureField[topmost].layer < textureField[i].layer)
				topmost = i;
	}
	selectedTextureField = topmost;
	return (topmost != -1);
};
bool gldRenderer::selectVariableFieldUnderMouse() {
	int topmost = -1;
	for (unsigned i = 0; i < variableField.size(); i++) {
		if (variableField[i].border.contains(input.mouse)) {
			if (topmost == -1 || variableField[topmost].layer < variableField[i].layer)
				topmost = i;
		}
	}
	selectedVariableField = topmost;
	return (topmost != -1);
};
unsigned gldRenderer::selectFieldUnderMouse() {
	unsigned result = FIELD_TYPE_NONE;
	unsigned layer = 0;
	if (selectModelFieldUnderMouse()) {
		result = FIELD_TYPE_MODEL;
		layer = modelField[selectedModelField].layer;
	}
	if (selectTextureFieldUnderMouse()) {
		if (textureField[selectedTextureField].layer >= layer) {
			layer = textureField[selectedTextureField].layer;
			result = FIELD_TYPE_TEXTURE;
			selectedModelField = -1;
		} else
			selectedTextureField = -1;
	}
	if (selectVariableFieldUnderMouse()) {
		if (variableField[selectedVariableField].layer >= layer) {
			layer = variableField[selectedVariableField].layer;
			result = FIELD_TYPE_VALUE;
			selectedModelField = -1;
			selectedTextureField = -1;
		}
		else
			selectedVariableField = -1;
	}
	return result;
};
void gldRenderer::render() {
	glLoadIdentity();
	// MODELS
	for (unsigned i = 0; i < modelField.size(); i++)
		modelField[i].draw();
	// TEXTURES
	for (unsigned i = 0; i < textureField.size(); i++)
		textureField[i].render();
	// VARIABLES
	for (unsigned i = 0; i < variableField.size(); i++)
		variableField[i].draw();
	// menus
	popupTextureSelect.draw();
	popupModelSelect.draw();
	popupVariableSelect.draw();
};
void gldRenderer::mouseDown(mouseButton button) {
	for (unsigned i = 0; i < modelField.size(); i++) {
		modelField[i].onMouseDown(button);
	}
	if (!popupTextureSelect.isActive())
		selectedTextureField = -1;
	if (!popupModelSelect.isActive())
		selectedModelField = -1;
	if (!popupVariableSelect.isActive())
		selectedVariableField = -1;
	if (button == mbRight) {
		unsigned selected = selectFieldUnderMouse();
		if (selected == FIELD_TYPE_MODEL) {
			popupModelSelect.show(input.mouse.x, input.mouse.y);
		} else
		if (selected == FIELD_TYPE_TEXTURE) {
			popupTextureSelect.show(input.mouse.x, input.mouse.y);
		} else
		if (selected == FIELD_TYPE_VALUE) {
			popupVariableSelect.setCheckedOptions(&variableField[selectedVariableField].itemNumber);
			popupVariableSelect.show(input.mouse.x, input.mouse.y);
		}
	}
	if (button == mbLeft) {
		if (popupModelSelect.isActive()) {
			int selectedMenuItem = popupModelSelect.selectedItemNumber();
			if (selectedMenuItem != -1) {
				modelField[selectedModelField].showModel(selectedMenuItem);
				save();
				selectedModelField = -1;
			}
		}
		else
		if (popupTextureSelect.isActive()) {
			int selectedMenuItem = popupTextureSelect.selectedItemNumber();
			if (selectedMenuItem != -1) {
				textureField[selectedTextureField].showTexture(selectedMenuItem);
				save();
				selectedTextureField = -1;
			}
		}
		else
		if (popupVariableSelect.isActive()) {
			int check = popupVariableSelect.checkedItem();
			if (check != -1) {
				variableField[selectedVariableField].setItem(check, true);
				save();
			}
			else {
				int uncheck = popupVariableSelect.uncheckedItem();
				if (uncheck != -1) {
					variableField[selectedVariableField].setItem(uncheck, false);
					save();
				}
			}
		}
	}
	requestRender();
};
void gldRenderer::mouseUp(mouseButton button) {
	for (unsigned i = 0; i < modelField.size(); i++) {
		modelField[i].onMouseUp(button);
	}
	save();
};
void gldRenderer::mouseMove(int x, int y) {
	for (unsigned i = 0; i < modelField.size(); i++) {
		modelField[i].onMouseMove(x, y);
	}
};
void gldRenderer::mouseWheel(signed short direction) {
	for (unsigned i = 0; i < modelField.size(); i++) {
		modelField[i].onMouseWheel(direction);
	}
	save();
};
void gldRenderer::clearFields() {
	modelField.clear();
	textureField.clear();
	variableField.clear();
};
bool gldRenderer::save() {
	ConfigFieldRecord * cField;
	for (unsigned i = 0; i < modelField.size(); i++) {
		if (configLoader.field.size() <= modelField[i].layer) 
			return false;
		cField = &configLoader.field[modelField[i].layer];
		cField->param_d.clear();
		cField->param_d.push_back(modelField[i].hang);
		cField->param_d.push_back(modelField[i].vang);
		cField->param_d.push_back(modelField[i].dist);
		cField->param_str = modelField[i].getModelCaption();
	}
	for (unsigned i = 0; i < textureField.size(); i++) {
		if (configLoader.field.size() <= textureField[i].layer)
			return false;
		cField = &configLoader.field[textureField[i].layer];
		cField->param_str = textureField[i].getTextureCaption();
	}
	for (unsigned i = 0; i < variableField.size(); i++) {
		if (configLoader.field.size() <= variableField[i].layer)
			return false;
		cField = &configLoader.field[variableField[i].layer];
		cField->param_i.clear();
		for (unsigned l = 0; l < variableField[i].itemNumber.size(); l++)
			cField->param_i.push_back(variableField[i].itemNumber[l]);
	}
	configLoader.save();
	return true;
};
bool gldRenderer::load() {
	clearFields();
	bool found = false;
	for (unsigned i = 0; i < configLoader.field.size(); i++) {
		const ConfigFieldRecord cField = configLoader.field[i];
		if (cField.type == FIELD_TYPE_MODEL) {
			ModelView MV(cField.border);
			MV.setModelList(&modelList);
			MV.layer = i;
			if (cField.param_str != "") {
				found = false;
				for (unsigned m = 0; m < modelList.size(); m++) {
					if (modelList[m].getCaption() == cField.param_str) {
						MV.showModel(m);
						found = true;
						break;
					}
				}
				if (!found)
					MV.waitingForModelCaption = cField.param_str;
			}
			if ( cField.param_d.size() >= 3 ) {
				MV.hang = cField.param_d[0];
				MV.vang = cField.param_d[1];
				MV.dist = cField.param_d[2];
			} else {
				std::cout << "WHAY? " << cField.param_str << std::endl;
			}
			modelField.push_back(MV);
		} else
		if (cField.type == FIELD_TYPE_TEXTURE) {
			TextureView TV;
			TV.setBorder(cField.border);
			TV.setTextureList(&textureList);
			TV.layer = i;
			found = false;
			for (unsigned tl = 0; tl < textureList.size(); tl++) {
				if (textureList[tl].getCaption() == cField.param_str) {
					TV.showTexture(tl);
					found = true;
					break;
				}
			}
			if (!found)
				TV.waitingForTextureCaption = cField.param_str;
			textureField.push_back(TV);
		} else
		if (cField.type == FIELD_TYPE_VALUE) {
			VariableView VV;
			VV.setBorder(cField.border);
			VV.setItemList(&variableItemList);
			VV.layer = i;
			for (unsigned l = 0; l < cField.param_i.size(); l++)
				VV.setItem(cField.param_i[l], true);
			variableField.push_back(VV);
		}
	}
	return true;
};
bool gldRenderer::addTexture(const char * caption, GLuint textureID) {
	TextureObject TE(caption, textureID);
	textureList.push_back(TE);
	popupTextureSelect.addItem(caption);
	for (unsigned i = 0; i < textureField.size(); i++) {
		textureField[i].setTextureList(&textureList);
		if (textureField[i].waitingForTextureCaption == caption)
			textureField[i].showTexture(textureList.size()-1);
	}
	return true;
};
bool gldRenderer::addValues(const char * caption, const char * formatString, void * data[]) {
	vwLine line;
	line.create(formatString, data);
	vwItem item;
	item.addLine(line);
	variableItemList.push_back(item);
	popupVariableSelect.addItem(caption);
	for (unsigned i = 0; i < variableField.size(); i++)
		variableField[i].setItemList(&variableItemList);
	return true;
};
bool gldRenderer::addValArray(const char* caption, void * data[], const char* format, int length, int lineLength) {
	
	vwItem item;
	vwLine firstLine;
	firstLine.setText(caption);
	item.addLine(firstLine);
	if (lineLength <= 0)
		lineLength = length;
	for (unsigned i = 0; i < length; i+=lineLength ) {
		vwLine line;
		if (length < i + lineLength)
			line.printArray(data, i, length % lineLength, format);
		else
			line.printArray(data, i, lineLength, format);
		item.addLine(line);
		std::cout << line.getText() << std::endl;
	}
	variableItemList.push_back(item);
	popupVariableSelect.addItem(caption);
	for (unsigned i = 0; i < variableField.size(); i++)
		variableField[i].setItemList(&variableItemList);
	return true;
};
bool gldRenderer::addModel(const char * caption, const unsigned count, const GLuint vboid, GLenum type) {
	for (unsigned i = 0; i < modelList.size(); i++)
	if (modelList[i].getCaption() == caption) {
		modelList[i].set(caption, count, vboid, type);
		return false;
	}
	ModelObject MO;
	MO.set(caption, count, vboid, type);
	modelList.push_back(MO);
	popupModelSelect.addItem(caption);
	for (unsigned i = 0; i < modelField.size(); i++) {
		modelField[i].setModelList(&modelList);
		if (modelField[i].waitingForModelCaption == caption)
			modelField[i].showModel(modelList.size() - 1);
	}
	return true;
};
ModelObject * gldRenderer::findModel(const char * caption) {
	for (unsigned i = 0; i < modelList.size(); i++)
	if (modelList[i].getCaption() == caption)
		return &modelList[i];
	return NULL;
}
void gldRenderer::addModelEdges(const char * caption, const GLenum mode, const unsigned count, const GLuint indices, GLenum type) {
	ModelObject * model = findModel(caption);
	if (model != NULL)
		model->setIndices(mode, count, indices, type);
};
void gldRenderer::addModelTexture(const char * caption, const GLuint texture, const GLuint coordinates, GLenum type) {
	ModelObject * model = findModel(caption);
	if (model != NULL)
		model->setTexture(texture, coordinates, type);
};
void gldRenderer::addModelData(const char * caption, float* data, float minValue, float maxValue) {
	ModelObject * model = findModel(caption);
	if (model != NULL)
		model->setData(data, minValue, maxValue);
};

///		T E X T U R E   V I E W		///
TextureObject::TextureObject(std::string C, GLuint ID) {
	caption = C;
	set(ID);
};
void TextureObject::set(GLuint ID) {
	bufferID = ID;
	glBindTexture(GL_TEXTURE_2D, bufferID);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	ratio = (float)width / height;
};
void TextureObject::render(Box frame) {
	if (bufferID == 0)
		return;
	float frameRatio = (float)frame.width / frame.height;
	Box texbox = frame;
	if (ratio > frameRatio) {
		texbox.width = frame.width;
		texbox.height = frame.width / ratio;
		texbox.move(0, (frame.height - texbox.height) / 2);
	}
	else {
		texbox.height = frame.height;
		texbox.width = frame.height * ratio;
		texbox.move((frame.width - texbox.width) / 2, 0);
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, bufferID);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(texbox.left - 1, texbox.top - 1);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(texbox.right, texbox.top - 1);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(texbox.right, texbox.bottom);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(texbox.left - 1, texbox.bottom);
	glEnd();
	glDisable(GL_TEXTURE_2D);
};

TextureObject * TextureView::getTexture() {
	if (index != -1 && list != NULL && index < list->size())
		return &(list->operator[](index));
	return NULL;
};
std::string TextureView::getTextureCaption() {
	TextureObject * texture = getTexture();
	if (texture != NULL)
		return texture->getCaption();
	return "";
};
void TextureView::showTexture(int id) {
	index = id;
	waitingForTextureCaption = "";
};
void TextureView::setTextureList(std::vector<TextureObject> * pointer) {
	list = pointer;
};
void TextureView::render() {
	drawBackground(0.04f, 0.04f, 0.04f);
	drawBorder(0.1f, 0.1f, 0.1f);
	TextureObject * texture = getTexture();
	if (texture != NULL)
		return texture->render(border);
};

///		M O D E L   V I E W		///
ModelObject::ModelObject() {
	set("", 0, 0, GL_FLOAT); 
	data.show = false;
	edges.show = false;
	texture.show = false;
};
void ModelObject::set(std::string C, unsigned N, unsigned VID, GLenum type) {
	caption = C;
	vertice.bid = VID;
	vertice.count = N;
	vertice.type = type;
	calculateTransformation();
};
void ModelObject::setData(float* P, float min, float max) {
	data.values = P;
	data.minValue = min;
	data.maxValue = max;
	data.normalized = (min == 0.0f) && (max == 1.0f);
	data.show = (P != NULL);
};
void ModelObject::setIndices(const GLenum mode, const unsigned count, const GLuint indices, GLenum type) {
	edges.bid = indices;
	edges.count = count;
	edges.mode = mode;
	edges.type = type;
	edges.show = (count != 0) && (indices != 0);
};
void ModelObject::setTexture(const GLuint tex, const GLuint coords, GLenum type) {
	texture.id = tex;
	texture.coords = coords;
	texture.type = type;
	texture.show = (tex != 0) && (coords != 0);
};
void ModelObject::calculateTransformation() {
	if (vertice.bid == 0) 
		return;
	glBindBuffer(GL_ARRAY_BUFFER, vertice.bid);
	GLfloat* vert = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	GLfloat maxpos[3], minpos[3];
	memcpy(maxpos, vert, 3 * sizeof(GLfloat));
	memcpy(minpos, vert, 3*sizeof(GLfloat));
	for (unsigned v = 1; v < vertice.count; v++) {
		GLfloat position[3];
		memcpy(position, (vert + 3 * v), 3 * sizeof(GLfloat));
		for (unsigned c = 0; c < 3; c++) {
			if (position[c] < minpos[c])
				minpos[c] = position[c];
			if (position[c] > maxpos[c])
				maxpos[c] = position[c];
		}
	}
	GLfloat delta[3], maxDelta = 0;
	for (unsigned i = 0; i < 3; i++) {
		delta[i] = maxpos[i] - minpos[i];
		if (delta[i] > maxDelta) 
			maxDelta = delta[i];
	}
	factor.scale = 1.0f / maxDelta;
	for (unsigned i = 0; i < 3; i++) {
		factor.translate[i] = -minpos[i] - delta[i] / 2;
	}
};
float ModelObject::normalizeValue(float value) {
	return (value - data.minValue) / (data.maxValue - data.minValue);
};
void ModelObject::renderPoints() {
	glPointSize(4.0f);
	glColor3f(0.9f, 0.9f, 0.9f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vertice.bid);
	glVertexPointer(3, vertice.type, 0, NULL);
	glDrawArrays(GL_POINTS, 0, vertice.count);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
};
void ModelObject::renderColoredPoints() {
	if (!data.show)
		return;
	glPointSize(6.0f);
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vertice.bid);
	glVertexPointer(3, vertice.type, 0, NULL);
	glEnable(GL_POINT_SMOOTH);
	for (unsigned i = 0; i < vertice.count; i++) {
		float value = data.values[i];
		if (!data.normalized)
			value = normalizeValue(value);
		setColorByValue(value);
		glDrawArrays(GL_POINTS, i, 1);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
};
void ModelObject::renderTextured() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vertice.bid);
	glVertexPointer(3, vertice.type, 0, NULL);
	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, texture.coords);
	glTexCoordPointer(2, texture.type, 0, NULL);

	glBindTexture(GL_TEXTURE_2D, texture.id);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edges.bid);
	glDrawElements(edges.mode, edges.count, edges.type, 0);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
};
void ModelObject::renderEdges() {
	glLineWidth(1.0f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vertice.bid);
	glVertexPointer(3, vertice.type, 0, NULL);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edges.bid);
	glDrawElements(edges.mode, edges.count, edges.type, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
};
void ModelObject::render() {
	if (vertice.bid == 0)
		return;
	glPushMatrix();
	glScalef(factor.scale, factor.scale, factor.scale);
	glTranslatef(factor.translate[0], factor.translate[1], factor.translate[2]);
	if (edges.show) {
		if (texture.show)
			renderTextured();
		else
			renderEdges();
	} else 
		renderPoints();
	if (data.show)
		renderColoredPoints();
	glPopMatrix();
};

ModelView::ModelView() { 
	modelList = NULL; 
	modelListIndex = -1; 
	hang = PI / 5.0;
	vang = PI / 4.0;
	dist = 5.0;
};
ModelView::ModelView(Box cBorder) { 
	border = cBorder; 
	modelList = NULL; 
	modelListIndex = -1; 
	hang = PI / 5.0;
	vang = PI / 4.0;
	dist = 5.0;
};
ModelObject * ModelView::getModel() {
	if (modelListIndex != -1 && modelList != NULL && modelListIndex < modelList->size())
		return &(modelList->operator[](modelListIndex));
	return NULL;
};
void ModelView::showModel(int index) {
	modelListIndex = index;
	waitingForModelCaption = "";
	wasSelected = false;
}
std::string ModelView::getModelCaption() {
	ModelObject * model = getModel();
	if (model != NULL)
		return model->getCaption();
	return "";
};
void ModelView::onMouseMove(int x, int y) {
	if (wasSelected) {
		hang -= (double)(lastMousePos.x - x) / 100.0;
		vang += (double)(lastMousePos.y - y) / 100.0;
		lastMousePos.set(x,y);
	}
	if (vang < 0.01) { vang = 0.01; }
	if (vang > PI) vang = PI;
};
void ModelView::onMouseDown(mouseButton button) {
	if (button==mbLeft && border.contains(input.mouse)) {
		wasSelected = true;
		lastMousePos = input.mouse;
	}
};
void ModelView::onMouseUp(mouseButton button) {
	if (button == mbLeft)
		wasSelected = false;
};
void ModelView::onMouseWheel(signed short direction) {
	if (border.contains(input.mouse)) {
		if (direction > 0)
			dist *= 0.8;
		else
			dist *= 1.2;
	}
};
void ModelView::draw() {
	drawBackground(0.05f, 0.05f, 0.05f);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glViewport(border.left, canvas.height - border.top- border.height, border.width, border.height);
	gluPerspective(25, (float)border.width / border.height, 0.1, 100);
	GLfloat x = dist * sin(vang) * sin(hang);
	GLfloat y = dist * cos(vang);
	GLfloat z = -dist * sin(vang) * cos(hang);
	gluLookAt(x,y,z, 0,0,0, 0,1,0);
	glMatrixMode(GL_MODELVIEW);
	ModelObject * model = getModel();
	if (model != NULL)
		model->render();
	reloadProjection();
};