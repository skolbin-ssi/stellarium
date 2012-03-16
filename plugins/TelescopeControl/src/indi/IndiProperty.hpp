/*
 * Qt-based INDI wire protocol client
 * 
 * Copyright (C) 2010-2012 Bogdan Marinov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INDI_PROPERTY_HPP_
#define _INDI_PROPERTY_HPP_

#include <QDateTime>
#include <QHash>
#define QT_SHAREDPOINTER_TRACK_POINTERS 1
#include <QSharedPointer>
#include <QString>
#include <QXmlStreamReader>

#include "IndiTypes.hpp"
#include "IndiElement.hpp"

//! Base class for data structures for the attributes of a vector tag
class TagAttributes
{
public:
	TagAttributes (const QXmlStreamReader& xmlStream);
	
	//! Attempts to read the \b timestamp attribute of the current element.
	//! \returns a UTC datetime if the timestamp can be parsed, otherwise
	//! an invalid QDateTime object.
	static QDateTime readTimestampAttribute(const QXmlStreamAttributes& attributes);
	
	bool areValid;
	
	QString device;
	QString name;
	QString timeoutString;
	QDateTime timestamp;
	QString message;
	
	//INDI XML attributes
	static const char* VERSION;
	static const char* DEVICE;
	static const char* NAME;
	static const char* LABEL;
	static const char* GROUP;
	static const char* STATE;
	static const char* PERMISSION;
	static const char* TIMEOUT;
	static const char* TIMESTAMP;
	static const char* MESSAGE;
	static const char* RULE;
	
protected:
	QXmlStreamAttributes attributes;
};

//! Self-populating data structure for the attributes of a defintion tag.
//! This class should be used in its pure form only in LightProperty only.
//! For everything else, use StandardPropertyDefinitionAttributes and
//! SwitchPropertyDefinitionAttributes.
class BasicDefTagAttributes : public TagAttributes
{
public:
	BasicDefTagAttributes (const QXmlStreamReader& xmlReader);
	
	QString label;
	QString group;
	State state;
};

//! Self-populating data structure for the attributes of a defintion tag.
class StandardDefTagAttributes :
        public BasicDefTagAttributes
{
public:
	StandardDefTagAttributes (const QXmlStreamReader& xmlReader);
	
	Permission permission;
};

//! Self-populating data structure for the attributes of a defintion tag.
class DefSwitchTagAttributes :
        public StandardDefTagAttributes
{
public:
	DefSwitchTagAttributes (const QXmlStreamReader& xmlReader);
	
	SwitchRule rule;
};

class SetTagAttributes : public TagAttributes
{
public:
	SetTagAttributes (const QXmlStreamReader& xmlReader);
	
	bool stateChanged;
	State state;
};

//! Base class of all property classes.
class Property : public QObject
{
	Q_OBJECT
public:
	enum PropertyType {
		TextProperty,
		NumberProperty,
		SwitchProperty,
		LightProperty,
		BlobProperty
	};

	Property(const QString& propertyName,
	         State propertyState,
	         Permission accessPermission,
	         const QString& propertyLabel = QString(),
	         const QString& propertyGroup = QString(),
	         const QDateTime& firstTimestamp = QDateTime());
	Property(const BasicDefTagAttributes& attributes);
	virtual ~Property();
	PropertyType getType() const;
	QString getName() const;
	QString getLabel() const;
	QString getGroup() const;
	QString getDevice() const;
	bool isReadable() const;
	bool isWritable() const;
	Permission getPermission() const;
	void setState(State newState);
	State getCurrentState() const;
	QDateTime getTimestamp() const;
	qint64 getTimestampInMilliseconds() const;
	virtual void addElement(Element* element) = 0;
	virtual int elementCount() const;
	virtual QStringList getElementNames() const;
	
	virtual void update(const QHash<QString,QString>& newValues,
	                    SetTagAttributes attributes);
	
	//! \todo Think of a better way of passing the new values.
	//! \todo Decide if it should perform validation.
	virtual void send(const QHash<QString,QString>& newValues);
	
	//! Name of the property vector definition element.
	virtual const char* defVectorTag() const = 0;
	//! Name of the property vector update-from-device element.
	virtual const char* setVectorTag() const = 0;
	//! Name of the property vector send-new-values element.
	virtual const char* newVectorTag() const = 0;
	//! Name of the property element definition element.
	virtual const char* defElementTag() const = 0;
	//! Name of the property element container element.
	virtual const char* oneElementTag() const = 0;
	
	// INDI tag names
	static const char* T_DEF_TEXT_VECTOR;
	static const char* T_DEF_NUMBER_VECTOR;
	static const char* T_DEF_SWITCH_VECTOR;
	static const char* T_DEF_LIGHT_VECTOR;
	static const char* T_DEF_BLOB_VECTOR;
	static const char* T_SET_TEXT_VECTOR;
	static const char* T_SET_NUMBER_VECTOR;
	static const char* T_SET_SWITCH_VECTOR;
	static const char* T_SET_LIGHT_VECTOR;
	static const char* T_SET_BLOB_VECTOR;
	static const char* T_NEW_TEXT_VECTOR;
	static const char* T_NEW_NUMBER_VECTOR;
	static const char* T_NEW_SWITCH_VECTOR;
	static const char* T_NEW_BLOB_VECTOR;
	static const char* T_DEF_TEXT;
	static const char* T_DEF_NUMBER;
	static const char* T_DEF_SWITCH;
	static const char* T_DEF_LIGHT;
	static const char* T_DEF_BLOB;
	static const char* T_ONE_TEXT;
	static const char* T_ONE_NUMBER;
	static const char* T_ONE_SWITCH;
	static const char* T_ONE_LIGHT;
	static const char* T_ONE_BLOB;
	
signals:
	//! Emitted when the property is updated with new values from the device.
	void newValuesReceived();
	//! Emitted when the property needs to send new values to the device.
	void valuesToSend(const QByteArray& indiCode);

protected:
	//! Sets the timestamp value. If necessary, reinterprets the data as UTC.
	//! If the argument is not a valid QDateTime, uses the current moment.
	void setTimestamp(const QDateTime& timestamp);
	//! Property type
	PropertyType type;

	//! Name used to identify the property internally.
	QString name;
	//! Human-readable label used to represent the property in the GUI.
	//! If not specified, the value of #name is used.
	QString label;
	//! Group name.
	QString group;
	//! Device name.
	QString device;
	//! Permission limiting client-side actions on this property.
	Permission permission;
	//! Current state of the property
	State state;
	//! Time in seconds before the next expected change in value.
	double timeout;
	//! Time of the last change (UTC)
	QDateTime timestamp;
	
	//! Property elements.
	QHash<QString,Element*> elements;
	
	//! Autogenerated string template for the "newXVector" start tag.
	QString newVectorStartTag;
	//! Autogenerated "newXVector" end tag.
	QString newVectorEndTag;
	//!
	QString oneElementTemplate;
	void initTagTemplates();
};

typedef QSharedPointer<Property> PropertyP;

//! A vector/array of string values (TextElement-s).
class TextProperty : public Property
{
public:
	TextProperty(const QString& propertyName,
	             State propertyState,
	             Permission accessPermission,
	             const QString& propertyLabel = QString(),
	             const QString& propertyGroup = QString(),
	             const QDateTime& timestamp = QDateTime());
	TextProperty(const StandardDefTagAttributes& attributes);
	~TextProperty();

	void addElement(Element* element);
	void addElement(TextElement* element);
	TextElement* getElement(const QString& name);

	const char* defVectorTag() const {return T_DEF_TEXT_VECTOR;}
	const char* defElementTag() const {return T_DEF_TEXT;}
	const char* setVectorTag() const {return T_SET_TEXT_VECTOR;}
	const char* newVectorTag() const {return T_NEW_TEXT_VECTOR;}
	const char* oneElementTag() const {return T_ONE_TEXT;}
};

typedef QSharedPointer<TextProperty> TextPropertyP;

//! A vector/array of numeric values (NumberElement-s).
class NumberProperty : public Property
{
public:
	//! \todo Do I need a device name field?
	//! \todo More stuff may need to be added to the constructor's arguments.
	NumberProperty(const QString& propertyName,
	               State propertyState,
	               Permission accessPermission,
	               const QString& propertyLabel = QString(),
	               const QString& propertyGroup = QString(),
	               const QDateTime& timestamp = QDateTime());
	NumberProperty(const StandardDefTagAttributes& attributes);
	virtual ~NumberProperty();

	void addElement(Element* element);
	void addElement(NumberElement* element);
	NumberElement* getElement(const QString& name);
	
	const char* defVectorTag() const {return T_DEF_NUMBER_VECTOR;}
	const char* defElementTag() const {return T_DEF_NUMBER;}
	const char* setVectorTag() const {return T_SET_NUMBER_VECTOR;}
	const char* newVectorTag() const {return T_NEW_NUMBER_VECTOR;}
	const char* oneElementTag() const {return T_ONE_NUMBER;}
};

typedef QSharedPointer<NumberProperty> NumberPropertyP;

//! A vector/array of switches (boolean SwitchElement-s)
class SwitchProperty : public Property
{
public:
	SwitchProperty(const QString& propertyName,
	               State propertyState,
	               Permission accessPermission,
	               SwitchRule switchRule,
	               const QString& propertyLabel = QString(),
	               const QString& propertyGroup = QString(),
	               const QDateTime& timestamp = QDateTime());
	SwitchProperty(const DefSwitchTagAttributes& attributes);
	virtual ~SwitchProperty();

	SwitchRule getSwitchRule() const;

	void addElement(Element* element);
	void addElement(SwitchElement* element);
	SwitchElement* getElement(const QString& name);
	
	void send(const QHash<QString,QString>& newValues);
	void send(const QHash<QString, bool>& newValues);
	
	const char* defVectorTag() const {return T_DEF_SWITCH_VECTOR;}
	const char* defElementTag() const {return T_DEF_SWITCH;}
	const char* setVectorTag() const {return T_SET_SWITCH_VECTOR;}
	const char* newVectorTag() const {return T_NEW_SWITCH_VECTOR;}
	const char* oneElementTag() const {return T_ONE_SWITCH;}

private:
	SwitchRule rule;
};

typedef QSharedPointer<SwitchProperty> SwitchPropertyP;

//! A vector/array of lights (LightElement-s).
class LightProperty : public Property
{
public:
	//! Lights are always read-only
	LightProperty(const QString& propertyName,
	              State propertyState,
	              const QString& propertyLabel = QString(),
	              const QString& propertyGroup = QString(),
	              const QDateTime& timestamp = QDateTime());
	LightProperty(const BasicDefTagAttributes& attributes);
	~LightProperty();

	void addElement(Element* element);
	void addElement(LightElement* element);
	LightElement* getElement(const QString& name);
	
	const char* defVectorTag() const {return T_DEF_LIGHT_VECTOR;}
	const char* defElementTag() const {return T_DEF_LIGHT;}
	const char* setVectorTag() const {return T_SET_LIGHT_VECTOR;}
	//! Returns an empty string.
	const char* newVectorTag() const {return "";}
	const char* oneElementTag() const {return T_ONE_LIGHT;}
};

typedef QSharedPointer<LightProperty> LightPropertyP;

//! A vector/array of BLOBs (BlobElement-s).
class BlobProperty : public Property
{
public:
	BlobProperty(const QString& propertyName,
	             State propertyState,
	             Permission accessPermission,
	             const QString& propertyLabel = QString(),
	             const QString& propertyGroup = QString(),
	             const QDateTime& timestamp = QDateTime());
	BlobProperty(const StandardDefTagAttributes& attributes);
	~BlobProperty();

	void addElement(Element* element);
	void addElement(BlobElement* element);
	BlobElement* getElement(const QString& name);
	
	const char* defVectorTag() const {return T_DEF_BLOB_VECTOR;}
	const char* defElementTag() const {return T_DEF_BLOB;}
	const char* setVectorTag() const {return T_SET_BLOB_VECTOR;}
	const char* newVectorTag() const {return T_SET_NUMBER_VECTOR;}
	const char* oneElementTag() const {return T_ONE_BLOB;}

	//! Reimplemented from Property::update(), ignores the newValues param.
	void update(const QHash<QString, QString>& newValues,
	            SetTagAttributes attributes);
};

typedef QSharedPointer<BlobProperty> BlobPropertyP;

#endif//_INDI_PROPERTY_HPP_
