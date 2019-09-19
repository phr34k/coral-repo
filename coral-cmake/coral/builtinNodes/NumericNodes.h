// <license>
// Copyright (C) 2011 Andrea Interguglielmi, All rights reserved.
// This file is part of the coral repository downloaded from http://code.google.com/p/coral-repo.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
// 
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// </license>


#ifndef CORAL_NUMERICNODES_H
#define CORAL_NUMERICNODES_H

#include "../src/Node.h"
#include "../src/Attribute.h"
#include "../src/NumericAttribute.h"
#include "../src/PassThroughAttribute.h"
#include "../src/StringAttribute.h"

namespace coral{
class Numeric;

class IntNode : public Node{
public:
	IntNode(const std::string &name, Node *parent);
		
private:
	NumericAttribute *_out;
};

class FloatNode : public Node{
public:
	FloatNode(const std::string &name, Node *parent);
	
private:
	NumericAttribute *_out;
};


class ConstantArray: public Node{
public:
	ConstantArray(const std::string &name, Node *parent);
	void updateSlice(Attribute *attribute, unsigned int slice);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	
private:
	NumericAttribute *_size;
	NumericAttribute *_constant;
	NumericAttribute *_array;
};

class ArraySize: public Node{
public:
	ArraySize(const std::string &name, Node *parent);
	void updateSlice(Attribute *attribute, unsigned int slice);

private:
	NumericAttribute *_array;
	NumericAttribute *_size;
};

class BuildArray: public Node{
public:
	BuildArray(const std::string &name, Node *parent);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void attributeSpecializationChanged(Attribute *attribute);
	void updateSlice(Attribute *attribute, unsigned int slice);
	void addNumericAttribute();
	
private:
	NumericAttribute *_array;
	void(BuildArray::*_selectedOperation)(const std::vector<Attribute*>&, int, Numeric*, unsigned int);
	void updateInt(const std::vector<Attribute*> &inAttrs, int arraySize, Numeric *array, unsigned int slice);
	void updateFloat(const std::vector<Attribute*> &inAttrs, int arraySize, Numeric *array, unsigned int slice);
};

class RangeArray: public Node{
public:
	RangeArray(const std::string &name, Node *parent);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void attributeSpecializationChanged(Attribute *attribute);
	void updateSlice(Attribute *attribute, unsigned int slice);
	
private:
	void(RangeArray::*_selectedOperation)(Numeric*, Numeric*, int, Numeric*, unsigned int);
	void updateInt(Numeric *start, Numeric *end, int steps, Numeric *array, unsigned int slice);
	void updateFloat(Numeric *start, Numeric *end, int steps, Numeric *array, unsigned int slice);
	NumericAttribute *_start;
	NumericAttribute *_end;
	NumericAttribute *_steps;
	NumericAttribute *_array;
};


class RangeLoop: public Node{
public:
	RangeLoop(const std::string &name, Node *parent);
	void attributeSpecializationChanged(Attribute *attribute);
	void updateSlice(Attribute *attribute, unsigned int slice);
	
private:
	NumericAttribute *_start;
	NumericAttribute *_end;
	NumericAttribute *_step;
	NumericAttribute *_valueInRange;
	void(RangeLoop::*_selectedOperation)(Numeric *, Numeric *, Numeric *, Numeric *, unsigned int);
	
	void updateFloat(Numeric *start, Numeric *end, Numeric *step, Numeric *out, unsigned int slice);
	void updateFloatArray(Numeric *start, Numeric *end, Numeric *step, Numeric *out, unsigned int slice);
	void updateInt(Numeric *start, Numeric *end, Numeric *step, Numeric *out, unsigned int slice);
	void updateIntArray(Numeric *start, Numeric *end, Numeric *step, Numeric *out, unsigned int slice);
};

class RandomNumber: public Node{
public:
	RandomNumber(const std::string &name, Node *parent);
	void attributeSpecializationChanged(Attribute *attribute);
	void updateSlice(Attribute *attribute, unsigned int slice);

private:
	NumericAttribute *_min;
	NumericAttribute *_max;
	NumericAttribute *_seed;
	NumericAttribute *_out;
	void(RandomNumber::*_selectedOperation)(Numeric *, Numeric *, Numeric *, unsigned int);
	
	void updateFloat(Numeric *min, Numeric *max, Numeric *out, unsigned int slice);
	void updateFloatArray(Numeric *min, Numeric *max, Numeric *out, unsigned int slice);
	void updateInt(Numeric *min, Numeric *max, Numeric *out, unsigned int slice);
	void updateIntArray(Numeric *min, Numeric *max, Numeric *out, unsigned int slice);
};

class ArrayIndices: public Node{
public:
	ArrayIndices(const std::string &name, Node *parent);
	void updateSlice(Attribute *attribute, unsigned int slice);
	
private:
	NumericAttribute *_array;
	NumericAttribute *_indices;
};

class GetArrayElement: public Node{
public:
	GetArrayElement(const std::string &name, Node *parent);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void attributeSpecializationChanged(Attribute *attribute);
	void updateSlice(Attribute *attribute, unsigned int slice);	

private:
	NumericAttribute *_array;
	NumericAttribute *_index;
	NumericAttribute *_element;
	void(GetArrayElement::*_selectedOperation)(Numeric *, const std::vector<int> &, Numeric *, unsigned int);
	
	void updateInt(Numeric *array, const std::vector<int> &index, Numeric *element, unsigned int slice);
	void updateFloat(Numeric *array, const std::vector<int> &index, Numeric *element, unsigned int slice);
};

class SetArrayElement: public Node{
public:
	SetArrayElement(const std::string &name, Node *parent);
	void updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB);
	void attributeSpecializationChanged(Attribute *attribute);
	void updateSlice(Attribute *attribute, unsigned int slice);	
	
private:
	NumericAttribute *_array;
	NumericAttribute *_index;
	NumericAttribute *_element;
	NumericAttribute *_outArray;
	void(SetArrayElement::*_selectedOperation)(Numeric *, const std::vector<int> &, Numeric *, Numeric *, unsigned int);

	void updateInt(Numeric *array, const std::vector<int> &index, Numeric *element, Numeric *outArray, unsigned int slice);
	void updateFloat(Numeric *array, const std::vector<int> &index, Numeric *element, Numeric *outArray, unsigned int slice);
};

class SetSimulationStep: public Node{
public:
	SetSimulationStep(const std::string &name, Node *parent);
	void attributeSpecializationChanged(Attribute *attribute);
	void updateSlice(Attribute *attribute, unsigned int slice);
	void resizedSlices(unsigned int slices);

	
private:
	StringAttribute *_storageKey;
	NumericAttribute *_data;
	NumericAttribute *_result;
	void(SetSimulationStep::*_selectedOperation)(const std::string &, Numeric *, Numeric *, unsigned int );

	void updateInt(const std::string &storageKey, Numeric *data, Numeric *result, unsigned int slice);
	void updateFloat(const std::string &storageKey, Numeric *data, Numeric *result, unsigned int slice);
};

class GetSimulationStep: public Node{
public:
	GetSimulationStep(const std::string &name, Node *parent);
	void updateSlice(Attribute *attribute, unsigned int slice);
	void attributeSpecializationChanged(Attribute *attribute);
	
private:
	StringAttribute *_storageKey;
	NumericAttribute *_source;
	NumericAttribute *_step;
	NumericAttribute *_data;
	void(GetSimulationStep::*_selectedOperation)(const std::string &, int, Numeric *, Numeric *, unsigned int);

	void updateInt(const std::string &storageKey, int step, Numeric *source, Numeric *data, unsigned int slice);
	void updateFloat(const std::string &storageKey, int step, Numeric *source, Numeric *data, unsigned int slice);
};



}
#endif


