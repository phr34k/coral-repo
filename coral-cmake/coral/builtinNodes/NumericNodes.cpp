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

#include <map>
#include <cmath>

#ifdef CORAL_PARALLEL_TBB
	#include <tbb/mutex.h>
#endif

#include "NumericNodes.h"
#include "../src/Numeric.h"
#include "../src/containerUtils.h"
#include "../src/mathUtils.h"
#include "../src/stringUtils.h"
#include "../src/Command.h"

#include <ImathEuler.h>

using namespace coral;

namespace {

int findMinorNumericSize(NumericAttribute *attrs[], int numAttrs, unsigned int slice){
	int minorSize = -1;
	for(int i = 0; i < numAttrs; ++i){
		if(attrs[i]->input()){
			Numeric *num = attrs[i]->value();
			int size = num->sizeSlice(slice);
			if(size < minorSize || minorSize == -1){
				minorSize = size;
			}
		}
	}
	if(minorSize == -1){
		minorSize = 0;
	}
	
	return minorSize;
}

std::map<std::string, Numeric> _globalNumericStorage;

#ifdef CORAL_PARALLEL_TBB
	tbb::mutex _globalMutex;
#endif
}


IntNode::IntNode(const std::string &name, Node* parent): Node(name, parent){
	setSliceable(true);

	_out = new NumericAttribute("out", this);
	addOutputAttribute(_out);
	
	setAttributeAllowedSpecialization(_out, "Int");
	
	_out->outValue()->resize(1);
	_out->outValue()->setFloatValueAtSlice(0, 0, 0.0);
}

FloatNode::FloatNode(const std::string &name, Node* parent): Node(name, parent){
	setSliceable(true);

	_out = new NumericAttribute("out", this);
	addOutputAttribute(_out);
	
	setAttributeAllowedSpecialization(_out, "Float");
	
	_out->outValue()->resize(1);
	_out->outValue()->setIntValueAtSlice(0, 0, 0.0);
}



ConstantArray::ConstantArray(const std::string &name, Node *parent): Node(name, parent){
	setSliceable(true);

	_size = new NumericAttribute("size", this);
	_constant = new NumericAttribute("constant", this);
	_array = new NumericAttribute("array", this);
	
	addInputAttribute(_size);
	addInputAttribute(_constant);
	addOutputAttribute(_array);
	
	setAttributeAffect(_size, _array);
	setAttributeAffect(_constant, _array);
	
	std::vector<std::string> constantSpecializations;
	constantSpecializations.push_back("Int");
	constantSpecializations.push_back("Float");
	constantSpecializations.push_back("Vec3");
	constantSpecializations.push_back("Col4");
	constantSpecializations.push_back("Matrix44");
	
	std::vector<std::string> arraySpecializations;
	arraySpecializations.push_back("IntArray");
	arraySpecializations.push_back("FloatArray");
	arraySpecializations.push_back("Vec3Array");
	arraySpecializations.push_back("Col4Array");
	arraySpecializations.push_back("Matrix44Array");
	
	setAttributeAllowedSpecialization(_size, "Int");
	setAttributeAllowedSpecializations(_constant, constantSpecializations);
	setAttributeAllowedSpecializations(_array, arraySpecializations);
	
	addAttributeSpecializationLink(_constant, _array);
}

void ConstantArray::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() < specializationB.size()){
		std::vector<std::string> newSpecialization;
		
		for(int i = 0; i < specializationA.size(); ++i){
			newSpecialization.push_back(specializationA[i] + "Array");
		}
		
		specializationB = newSpecialization;
	}
	else{
		std::vector<std::string> newSpecialization;
		
		for(int i = 0; i < specializationB.size(); ++i){
			newSpecialization.push_back(stringUtils::replace(specializationB[i], "Array", ""));
		}
		
		specializationA = newSpecialization;
	}
}

void ConstantArray::updateSlice(Attribute *attribute, unsigned int slice){
	Numeric *constant = _constant->value();
	if(constant->type() != Numeric::numericTypeAny){
		int size = _size->value()->intValueAtSlice(slice, 0);
		if(size < 0){
			size = 0;
			_size->outValue()->setIntValueAtSlice(slice, 0, 0);
		}
		
		Numeric *array = _array->outValue();
		if(constant->type() == Numeric::numericTypeInt){
			int constantValue = constant->intValueAtSlice(slice, 0);
			std::vector<int> values(size);
			for(int i = 0; i < size; ++i){
				values[i] = constantValue;
			}
			array->setIntValuesSlice(slice, values);
		}
		else if(constant->type() == Numeric::numericTypeFloat){
			float constantValue = constant->floatValueAtSlice(slice, 0);
			std::vector<float> values(size);
			for(int i = 0; i < size; ++i){
				values[i] = constantValue;
			}
			array->setFloatValuesSlice(slice, values);
		}
        /*
		else if(constant->type() == Numeric::numericTypeVec3){
			Imath::V3f constantValue = constant->vec3ValueAtSlice(slice, 0);
			std::vector<Imath::V3f> values(size);
			for(int i = 0; i < size; ++i){
				values[i] = constantValue;
			}
			array->setVec3ValuesSlice(slice, values);
		}
		else if(constant->type() == Numeric::numericTypeCol4){
			Imath::Color4f constantValue = constant->col4ValueAtSlice(slice, 0);
			std::vector<Imath::Color4f> values(size);
			for(int i = 0; i < size; ++i){
				values[i] = constantValue;
			}
			array->setCol4ValuesSlice(slice, values);
		}
		else if(constant->type() == Numeric::numericTypeMatrix44){
			Imath::M44f constantValue = constant->matrix44ValueAtSlice(slice, 0);
			std::vector<Imath::M44f> values(size);
			for(int i = 0; i < size; ++i){
				values[i] = constantValue;
			}
			array->setMatrix44ValuesSlice(slice, values);
		}
        */
	}

	else{
		setAttributeIsClean(_array, false);
	}
}

ArraySize::ArraySize(const std::string &name, Node *parent): Node(name, parent){
	setSliceable(true);

	_array = new NumericAttribute("array", this);
	_size = new NumericAttribute("size", this);
	
	addInputAttribute(_array);
	addOutputAttribute(_size);
	
	setAttributeAffect(_array, _size);
	
	std::vector<std::string> arraySpecializations;
	arraySpecializations.push_back("IntArray");
	arraySpecializations.push_back("FloatArray");
	arraySpecializations.push_back("Vec3Array");
	arraySpecializations.push_back("Col4Array");
	arraySpecializations.push_back("Matrix44Array");
	
	setAttributeAllowedSpecializations(_array, arraySpecializations);
	setAttributeAllowedSpecialization(_size, "Int");
}

void ArraySize::updateSlice(Attribute *attribute, unsigned int slice){
	Numeric *array = _array->value();
	if(array->type() != Numeric::numericTypeAny){
		_size->outValue()->setIntValueAtSlice(slice, 0, array->sizeSlice(slice));
	}
	else{
		setAttributeIsClean(_size, false);
	}
}


BuildArray::BuildArray(const std::string &name, Node *parent): 
	Node(name, parent),
	_selectedOperation(0){
	setSliceable(true);

	setAllowDynamicAttributes(true);
	
	_array = new NumericAttribute("array", this);
	addOutputAttribute(_array);
}

void BuildArray::addNumericAttribute(){
	std::string numStr = stringUtils::intToString(inputAttributes().size());
	NumericAttribute *attr = new NumericAttribute("in" + numStr, this);
	addInputAttribute(attr);
	setAttributeAffect(attr, _array);
	
	std::vector<std::string> specialization;
	specialization.push_back("Int");
	specialization.push_back("Float");
	specialization.push_back("Vec3");
	specialization.push_back("Col4");
	specialization.push_back("Matrix44");
	
	setAttributeAllowedSpecializations(attr, specialization);
	
	addAttributeSpecializationLink(attr, _array);
	
	addDynamicAttribute(attr);
	updateAttributeSpecialization(attr);
}

void BuildArray::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() <= specializationB.size()){
		specializationB.resize(specializationA.size());
		for(int i = 0; i < specializationA.size(); ++i){
			specializationB[i] = specializationA[i] + "Array";
		}
	}
	else{
		specializationA.resize(specializationB.size());
		for(int i = 0; i < specializationB.size(); ++i){
			specializationA[i] = stringUtils::strip(specializationB[i], "Array");
		}
	}
}

void BuildArray::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _array){
		_selectedOperation = 0;
		
		Numeric::Type type = _array->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeIntArray){
				_selectedOperation = &BuildArray::updateInt;
			}
			else if(type == Numeric::numericTypeFloatArray){
				_selectedOperation = &BuildArray::updateFloat;
			}
            /*
			else if(type == Numeric::numericTypeVec3Array){
				_selectedOperation = &BuildArray::updateVec3;
			}
			else if(type == Numeric::numericTypeCol4Array){
				_selectedOperation = &BuildArray::updateCol4;
			}
			else if(type == Numeric::numericTypeMatrix44Array){
				_selectedOperation = &BuildArray::updateMatrix44;
			}
            */
		}
	}
}

void BuildArray::updateInt(const std::vector<Attribute*> &inAttrs, int arraySize, Numeric *array, unsigned int slice){
	std::vector<int> arrayValues(arraySize);
	for(int i = 0; i < arraySize; ++i){
		Numeric *inNum = (Numeric*)inAttrs[i]->value();
		arrayValues[i] = inNum->intValueAtSlice(slice, 0);
	}
	
	array->setIntValuesSlice(slice, arrayValues);
}

void BuildArray::updateFloat(const std::vector<Attribute*> &inAttrs, int arraySize, Numeric *array, unsigned int slice){
	std::vector<float> arrayValues(arraySize);
	for(int i = 0; i < arraySize; ++i){
		Numeric *inNum = (Numeric*)inAttrs[i]->value();
		arrayValues[i] = inNum->floatValueAtSlice(slice, 0);
	}
	
	array->setFloatValuesSlice(slice, arrayValues);
}

void BuildArray::updateSlice(Attribute *attribute, unsigned int slice){
	if(_selectedOperation){
		std::vector<Attribute*> inAttrs = inputAttributes();
		int arraySize = inAttrs.size();
		Numeric *array = _array->outValue();
		
		(this->*_selectedOperation)(inAttrs, arraySize, array, slice);
	}
}

RangeArray::RangeArray(const std::string &name, Node* parent):
Node(name, parent),
_selectedOperation(0){	
	setSliceable(true);

	_start = new NumericAttribute("start", this);
	_end = new NumericAttribute("end", this);
	_steps = new NumericAttribute("steps", this);
	_array = new NumericAttribute("array", this);
	
	addInputAttribute(_start);
	addInputAttribute(_end);
	addInputAttribute(_steps);
	addOutputAttribute(_array);
	
	setAttributeAffect(_start, _array);
	setAttributeAffect(_end, _array);
	setAttributeAffect(_steps, _array);
	
	std::vector<std::string> inSpecializations;
	inSpecializations.push_back("Int");
	inSpecializations.push_back("Float");
	
	std::vector<std::string> outSpecializations;
	outSpecializations.push_back("IntArray");
	outSpecializations.push_back("FloatArray");
	
	setAttributeAllowedSpecializations(_start, inSpecializations);
	setAttributeAllowedSpecializations(_end, inSpecializations);
	setAttributeAllowedSpecialization(_steps, "Int");
	setAttributeAllowedSpecializations(_array, outSpecializations);
	
	addAttributeSpecializationLink(_start, _array);
	addAttributeSpecializationLink(_end, _array);
}

void RangeArray::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(specializationA.size() <= specializationB.size()){
		specializationB.resize(specializationA.size());
		for(int i = 0; i < specializationA.size(); ++i){
			specializationB[i] = specializationA[i] + "Array";
		}
	}
	else{
		specializationA.resize(specializationB.size());
		for(int i = 0; i < specializationB.size(); ++i){
			specializationA[i] = stringUtils::strip(specializationB[i], "Array");
		}
	}
}

void RangeArray::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _array){
		_selectedOperation = 0;
		
		Numeric::Type type = _array->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeIntArray){
				_selectedOperation = &RangeArray::updateInt;
			}
			else if(type == Numeric::numericTypeFloatArray){
				_selectedOperation = &RangeArray::updateFloat;
			}
		}
	}
}

void RangeArray::updateInt(Numeric *start, Numeric *end, int steps, Numeric *array, unsigned int slice){
	int startValue = start->intValueAtSlice(slice, 0);
	int endValue = end->intValueAtSlice(slice, 0);
	
	float totalRange = endValue - startValue;
	int incrStep = totalRange / steps;
	int currentStep = startValue;
	std::vector<int> arrayValues(steps + 1);
	
	for(int i = 0; i < steps + 1; ++i){
		arrayValues[i] = currentStep;
		currentStep += incrStep;
	}
	
	array->setIntValuesSlice(slice, arrayValues);
}

void RangeArray::updateFloat(Numeric *start, Numeric *end, int steps, Numeric *array, unsigned int slice){
	float startValue = start->floatValueAtSlice(slice, 0);
	float endValue = end->floatValueAtSlice(slice, 0);
	
	float totalRange = endValue - startValue;
	float incrStep = totalRange / float(steps);
	float currentStep = startValue;
	std::vector<float> arrayValues(steps + 1);
	
	for(int i = 0; i < steps + 1; ++i){
		arrayValues[i] = currentStep;
		
		currentStep += incrStep;
	}
	
	array->setFloatValuesSlice(slice, arrayValues);
}

void RangeArray::updateSlice(Attribute *attribute, unsigned int slice){
	if(_selectedOperation){
		Numeric *start = _start->value();
		Numeric *end = _end->value();
		int steps = _steps->value()->intValueAtSlice(slice, 0);
		if(steps < 0){
			_steps->outValue()->setIntValueAtSlice(slice, 0, 0);
			steps = 0;
		}
		
		Numeric *array = _array->outValue();
		
		(this->*_selectedOperation)(start, end, steps, array, slice);
	}
}

RangeLoop::RangeLoop(const std::string &name, Node* parent): 
	Node(name, parent),
	_selectedOperation(0){	
	setSliceable(true);

	_start = new NumericAttribute("start", this);
	_end = new NumericAttribute("end", this);
	_step = new NumericAttribute("step", this);
	_valueInRange = new NumericAttribute("valueInRange", this);
	
	addInputAttribute(_start);
	addInputAttribute(_end);
	addInputAttribute(_step);
	addOutputAttribute(_valueInRange);
	
	setAttributeAffect(_start, _valueInRange);
	setAttributeAffect(_end, _valueInRange);
	setAttributeAffect(_step, _valueInRange);
	
	std::vector<std::string> specialization;
	specialization.push_back("Float");
	specialization.push_back("FloatArray");
	specialization.push_back("Int");
	specialization.push_back("IntArray");
	
	setAttributeAllowedSpecializations(_start, specialization);
	setAttributeAllowedSpecializations(_end, specialization);
	setAttributeAllowedSpecializations(_step, specialization);
	setAttributeAllowedSpecializations(_valueInRange, specialization);
	
	addAttributeSpecializationLink(_start, _valueInRange);
	addAttributeSpecializationLink(_end, _valueInRange);
	addAttributeSpecializationLink(_step, _valueInRange);
}

void RangeLoop::updateFloat(Numeric *start, Numeric *end, Numeric *step, Numeric *out, unsigned int slice){
	float startVal = start->floatValueAtSlice(slice, 0);
	float endVal = end->floatValueAtSlice(slice, 0);
	float stepVal = step->floatValueAtSlice(slice, 0);
	
	float outVal = fmod((stepVal + startVal), endVal);
	out->setFloatValueAtSlice(slice, 0, outVal);
}

void RangeLoop::updateFloatArray(Numeric *start, Numeric *end, Numeric *step, Numeric *out, unsigned int slice){
	NumericAttribute *attrs[] = {_start, _end, _step};
			
	int minorSize = findMinorNumericSize(attrs, 3, slice);
	if(minorSize < 1)
		minorSize = 1;
	
	std::vector<float> outVals(minorSize);
	for(int i = 0; i < minorSize; ++i){
		float startVal = start->floatValueAtSlice(slice, i);
		float endVal = end->floatValueAtSlice(slice, i);
		float stepVal = step->floatValueAtSlice(slice, i);

		outVals[i] = fmod((stepVal + startVal), endVal);
	}
	
	out->setFloatValuesSlice(slice, outVals);
}

void RangeLoop::updateInt(Numeric *start, Numeric *end, Numeric *step, Numeric *out, unsigned int slice){
	int startVal = start->intValueAtSlice(slice, 0);
	int endVal = end->intValueAtSlice(slice, 0);
	int stepVal = step->intValueAtSlice(slice, 0);

	int outVal = 0;
	if(endVal > 0){
		outVal = (stepVal + startVal) % endVal;
	}

	out->setIntValueAtSlice(slice, 0, outVal);
}

void RangeLoop::updateIntArray(Numeric *start, Numeric *end, Numeric *step, Numeric *out, unsigned int slice){
	NumericAttribute *attrs[] = {_start, _end, _step};
			
	int minorSize = findMinorNumericSize(attrs, 3, slice);
	if(minorSize < 1)
		minorSize = 1;
	
	std::vector<int> outVals(minorSize);
	for(int i = 0; i < minorSize; ++i){
		int startVal = start->intValueAtSlice(slice, i);
		int endVal = end->intValueAtSlice(slice, i);
		int stepVal = step->intValueAtSlice(slice, i);

		int outVal = 0;
		if(endVal > 0){
			outVal = (stepVal + startVal) % endVal;
		}

		outVals[i] = outVal;
	}
	
	out->setIntValuesSlice(slice, outVals);
}

void RangeLoop::attributeSpecializationChanged(Attribute *attribute){
	_selectedOperation = 0;
	
	std::vector<std::string> outSpec = _valueInRange->specialization();
	if(outSpec.size() == 1){
		std::string spec = outSpec[0];
		
		if(spec == "Float"){
			_selectedOperation = &RangeLoop::updateFloat;
		}
		else if(spec == "FloatArray"){
			_selectedOperation = &RangeLoop::updateFloatArray;
		}
		else if(spec == "Int"){
			_selectedOperation = &RangeLoop::updateInt;
		}
		else if(spec == "IntArray"){
			_selectedOperation = &RangeLoop::updateIntArray;
		}
	}
}

void RangeLoop::updateSlice(Attribute *attribute, unsigned int slice){
	if(_selectedOperation){
		Numeric *start = _start->value();
		Numeric *end = _end->value();
		Numeric *step = _step->value();
		Numeric *out = _valueInRange->outValue();
		
		(this->*_selectedOperation)(start, end, step, out, slice);
	}
}

RandomNumber::RandomNumber(const std::string &name, Node* parent): 
	Node(name, parent),
	_selectedOperation(0){	
	setSliceable(true);

	_min = new NumericAttribute("min", this);
	_max = new NumericAttribute("max", this);
	_seed = new NumericAttribute("seed", this);
	_out = new NumericAttribute("out", this);
	
	addInputAttribute(_min);
	addInputAttribute(_max);
	addInputAttribute(_seed);
	addOutputAttribute(_out);
	
	setAttributeAffect(_min, _out);
	setAttributeAffect(_max, _out);
	setAttributeAffect(_seed, _out);
	
	std::vector<std::string> specialization;
	specialization.push_back("Float");
	specialization.push_back("FloatArray");
	specialization.push_back("Int");
	specialization.push_back("IntArray");
	
	setAttributeAllowedSpecializations(_min, specialization);
	setAttributeAllowedSpecializations(_max, specialization);
	setAttributeAllowedSpecialization(_seed, "Int");
	setAttributeAllowedSpecializations(_out, specialization);
	
	addAttributeSpecializationLink(_min, _out);
	addAttributeSpecializationLink(_max, _out);
}

void RandomNumber::attributeSpecializationChanged(Attribute *attribute){
	_selectedOperation = 0;
	
	std::vector<std::string> outSpec = _out->specialization();
	if(outSpec.size() == 1){
		std::string spec = outSpec[0];
		
		if(spec == "Float"){
			_selectedOperation = &RandomNumber::updateFloat;
		}
		else if(spec == "FloatArray"){
			_selectedOperation = &RandomNumber::updateFloatArray;
		}
		else if(spec == "Int"){
			_selectedOperation = &RandomNumber::updateInt;
		}
		else if(spec == "IntArray"){
			_selectedOperation = &RandomNumber::updateIntArray;
		}
	}
	
}

void RandomNumber::updateFloat(Numeric *min, Numeric *max, Numeric *out, unsigned int slice){
	float minVal = min->floatValueAtSlice(slice, 0);
	float maxVal = max->floatValueAtSlice(slice, 0);

	srand(_seed->value()->intValueAtSlice(slice, 0));
	for(float i = minVal; i < maxVal; ++i){
		rand();
	}
	
	float outVal = ((float(rand()) / float(RAND_MAX)) * (maxVal - minVal)) + minVal;

	out->setFloatValueAtSlice(slice, 0, outVal);
}

void RandomNumber::updateFloatArray(Numeric *min, Numeric *max, Numeric *out, unsigned int slice){
	NumericAttribute *attrs[] = {_min, _max};
	
	int minorSize = findMinorNumericSize(attrs, 2, slice);
	if(minorSize < 1)
		minorSize = 1;
	
	srand(_seed->value()->intValueAtSlice(slice, 0));
	std::vector<float> outVals(minorSize);
	for(int i = 0; i < minorSize; ++i){
		float minVal = min->floatValueAtSlice(slice, i);
		float maxVal = max->floatValueAtSlice(slice, i);
		
		outVals[i] =  ((float(rand()) / float(RAND_MAX)) * (maxVal - minVal)) + minVal;
	}
	
	out->setFloatValuesSlice(slice, outVals);
}

void RandomNumber::updateInt(Numeric *min, Numeric *max, Numeric *out, unsigned int slice){
	int minVal = min->intValueAtSlice(slice, 0);
	int maxVal = max->intValueAtSlice(slice, 0);

	srand(_seed->value()->intValueAtSlice(slice, 0));
	for(int i = minVal; i < maxVal; ++i){
		rand();
	}

	int outVal = minVal + (int) (maxVal - minVal + 1)*(rand() / (RAND_MAX + 1.0));

	out->setIntValueAtSlice(slice, 0, outVal);
}

void RandomNumber::updateIntArray(Numeric *min, Numeric *max, Numeric *out, unsigned int slice){
	NumericAttribute *attrs[] = {_min, _max};
			
	int minorSize = findMinorNumericSize(attrs, 2, slice);
	if(minorSize < 1)
		minorSize = 1;
	
	srand(_seed->value()->intValueAtSlice(slice, 0));
	std::vector<int> outVals(minorSize);
	for(int i = 0; i < minorSize; ++i){
		int minVal = min->intValueAtSlice(slice, i);
		int maxVal = max->intValueAtSlice(slice, i);
		
		outVals[i] = minVal + (int) (maxVal - minVal + 1)*(rand() / (RAND_MAX + 1.0));
	}
	
	out->setIntValuesSlice(slice, outVals);
}

void RandomNumber::updateSlice(Attribute *attribute, unsigned int slice){
	if(_selectedOperation){
		Numeric *min = _min->value();
		Numeric *max = _max->value();
		Numeric *out = _out->outValue();
		
		(this->*_selectedOperation)(min, max, out, slice);
	}
}

ArrayIndices::ArrayIndices(const std::string &name, Node* parent): Node(name, parent){
	setSliceable(true);

	_array = new NumericAttribute("array", this);
	_indices = new NumericAttribute("indices", this);
	
	addInputAttribute(_array);
	addOutputAttribute(_indices);
	
	setAttributeAffect(_array, _indices);
	
	std::vector<std::string> arraySpec;
	arraySpec.push_back("IntArray");
	arraySpec.push_back("FloatArray");
	arraySpec.push_back("Vec3Array");
	arraySpec.push_back("Col4Array");
	arraySpec.push_back("Matrix44Array");
	
	setAttributeAllowedSpecializations(_array, arraySpec);
	setAttributeAllowedSpecialization(_indices, "IntArray");
}

void ArrayIndices::updateSlice(Attribute *attribute, unsigned int slice){
	int arraySize = _array->value()->sizeSlice(slice);
	std::vector<int> indices(arraySize);
	
	for(int i = 0; i < arraySize; ++i){
		indices[i] = i;
	}
	
	_indices->outValue()->setIntValuesSlice(slice, indices);
}

GetArrayElement::GetArrayElement(const std::string &name, Node *parent): 
	Node(name, parent),
	_selectedOperation(0){
	setSliceable(true);

	_array = new NumericAttribute("array", this);
	_index = new NumericAttribute("index", this);
	_element = new NumericAttribute("element", this);
	
	addInputAttribute(_array);
	addInputAttribute(_index);
	addOutputAttribute(_element);
	
	setAttributeAffect(_array, _element);
	setAttributeAffect(_index, _element);
	
	std::vector<std::string> arraySpec;
	arraySpec.push_back("IntArray");
	arraySpec.push_back("FloatArray");
	arraySpec.push_back("Vec3Array");
	arraySpec.push_back("Col4Array");
	arraySpec.push_back("Matrix44Array");
	
	std::vector<std::string> elementSpec;
	elementSpec.push_back("Int");
	elementSpec.push_back("IntArray");
	elementSpec.push_back("Float");
	elementSpec.push_back("FloatArray");
	elementSpec.push_back("Vec3");
	elementSpec.push_back("Vec3Array");
	elementSpec.push_back("Col4");
	elementSpec.push_back("Col4Array");
	elementSpec.push_back("Matrix44");
	elementSpec.push_back("Matrix44Array");
	
	std::vector<std::string> indexSpec;
	indexSpec.push_back("Int");
	indexSpec.push_back("IntArray");
	
	setAttributeAllowedSpecializations(_array, arraySpec);
	setAttributeAllowedSpecializations(_index, indexSpec);
	setAttributeAllowedSpecializations(_element, elementSpec);
	
	addAttributeSpecializationLink(_array, _element);
	addAttributeSpecializationLink(_index, _element);
}

void GetArrayElement::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(attributeA == _array){
		std::vector<std::string> newSpecA;
		for(int i = 0; i < specializationA.size(); ++i){
			std::string &specA = specializationA[i];
			std::string typeA = stringUtils::strip(specA, "Array");
			for(int j = 0; j < specializationB.size(); ++j){
				std::string typeB = stringUtils::strip(specializationB[j], "Array");
				if(typeA == typeB){
					newSpecA.push_back(specA);
					break;
				}
			}
		}
		
		std::vector<std::string> newSpecB;
		for(int i = 0; i < specializationB.size(); ++i){
			std::string &specB = specializationB[i];
			std::string typeB = stringUtils::strip(specB, "Array");
			for(int j = 0; j < specializationA.size(); ++j){
				std::string typeA = stringUtils::strip(specializationA[j], "Array");
				if(typeB == typeA){
					newSpecB.push_back(specB);
					break;
				}
			}
		}
		
		specializationA = newSpecA;
		specializationB = newSpecB;
	}
	else{
		if(specializationA.size() == 1){
			std::string specASuffix = "";
			if(stringUtils::endswith(specializationA[0], "Array")){
				specASuffix = "Array";
			}
			
			std::vector<std::string> newSpecB;
			for(int i = 0; i < specializationB.size(); ++i){
				std::string &specB = specializationB[i];
				
				std::string specBSuffix = "";
				if(stringUtils::endswith(specB, "Array")){
					specBSuffix = "Array";
				}
				
				if(specASuffix == specBSuffix){
					newSpecB.push_back(specB);
				}
			}
			
			specializationB = newSpecB;
		}
		else{
			std::vector<std::string> newSpecA;
			for(int i = 0; i < specializationB.size(); ++i){
				std::string &specB = specializationB[i];
				if(!stringUtils::endswith(specB, "Array")){
					newSpecA.push_back("Int");
					break;
				}
			}
			
			for(int i = 0; i < specializationB.size(); ++i){
				std::string &specB = specializationB[i];
				if(stringUtils::endswith(specB, "Array")){
					newSpecA.push_back("IntArray");
					break;
				}
			}
			
			specializationA = newSpecA;
		}
	}
}

void GetArrayElement::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _array){
		_selectedOperation = 0;
		
		Numeric::Type type = _array->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeIntArray){
				_selectedOperation = &GetArrayElement::updateInt;
			}
			else if(type == Numeric::numericTypeFloatArray){
				_selectedOperation = &GetArrayElement::updateFloat;
			}
            /*
			else if(type == Numeric::numericTypeVec3Array){
				_selectedOperation = &GetArrayElement::updateVec3;
			}
			else if(type == Numeric::numericTypeCol4Array){
				_selectedOperation = &GetArrayElement::updateCol4;
			}
			else if(type == Numeric::numericTypeMatrix44Array){
				_selectedOperation = &GetArrayElement::updateMatrix44;
			}
            */
		}
	}
}

void GetArrayElement::updateInt(Numeric *array, const std::vector<int> &index, Numeric *element, unsigned int slice){
	int size = index.size();
	element->resize(size);
	for(int i = 0; i < size; ++i){
		element->setIntValueAtSlice(slice, i, array->intValueAtSlice(slice, index[i]));
	}
}

void GetArrayElement::updateFloat(Numeric *array,  const std::vector<int> &index, Numeric *element, unsigned int slice){
	int size = index.size();
	element->resize(size);
	for(int i = 0; i < size; ++i){
		element->setFloatValueAtSlice(slice, i, array->floatValueAtSlice(slice, index[i]));
	}
}

void GetArrayElement::updateSlice(Attribute *attribute, unsigned int slice){
	if(_selectedOperation){
		Numeric *array = _array->value();
		std::vector<int> index = _index->value()->intValuesSlice(slice);
		Numeric *element = _element->outValue();
		
		(this->*_selectedOperation)(array, index, element, slice);
	}
}

SetArrayElement::SetArrayElement(const std::string &name, Node *parent):
Node(name, parent),
_selectedOperation(0){
	setSliceable(true);

	_array = new NumericAttribute("array", this);
	_index = new NumericAttribute("index", this);
	_element = new NumericAttribute("element", this);
	_outArray = new NumericAttribute("outArray", this);
	
	addInputAttribute(_array);
	addInputAttribute(_index);
	addInputAttribute(_element);
	addOutputAttribute(_outArray);
	
	setAttributeAffect(_array, _outArray);
	setAttributeAffect(_element, _outArray);
	setAttributeAffect(_index, _outArray);
	
	std::vector<std::string> elementSpecs;
	elementSpecs.push_back("Int");
	elementSpecs.push_back("IntArray");
	elementSpecs.push_back("Float");
	elementSpecs.push_back("FloatArray");
	elementSpecs.push_back("Vec3");
	elementSpecs.push_back("Vec3Array");
	elementSpecs.push_back("Col4");
	elementSpecs.push_back("Col4Array");
	elementSpecs.push_back("Matrix44");
	elementSpecs.push_back("Matrix44Array");
	
	std::vector<std::string> arraySpecs;
	arraySpecs.push_back("IntArray");
	arraySpecs.push_back("FloatArray");
	arraySpecs.push_back("Vec3Array");
	arraySpecs.push_back("Col4Array");
	arraySpecs.push_back("Matrix44Array");
	
	std::vector<std::string> indexSpec;
	indexSpec.push_back("Int");
	indexSpec.push_back("IntArray");

	setAttributeAllowedSpecializations(_array, arraySpecs);
	setAttributeAllowedSpecializations(_element, elementSpecs);
	setAttributeAllowedSpecializations(_index, indexSpec);
	setAttributeAllowedSpecializations(_outArray, arraySpecs);
	
	addAttributeSpecializationLink(_array, _outArray);
	addAttributeSpecializationLink(_element, _outArray);
}

void SetArrayElement::updateSpecializationLink(Attribute *attributeA, Attribute *attributeB, std::vector<std::string> &specializationA, std::vector<std::string> &specializationB){
	if(attributeA == _array){
		Node::updateSpecializationLink(attributeA, attributeB, specializationA, specializationB);
	}
	else{
		std::vector<std::string> newSpecA;
		std::vector<std::string> newSpecB;
		for(int i = 0; i < specializationA.size(); ++i){
			std::string &specA = specializationA[i];
			std::string typeA = stringUtils::strip(specA, "Array");
			for(int j = 0; j < specializationB.size(); ++j){
				std::string &specB = specializationB[j];
				std::string typeB = stringUtils::strip(specB, "Array");
				if(typeA == typeB){
					if(!containerUtils::elementInContainer(specB, newSpecB)){
						newSpecB.push_back(specB);
					}
					if(!containerUtils::elementInContainer(specA, newSpecA)){
						newSpecA.push_back(specA);
					}
				}
			}
		}

		specializationA = newSpecA;
		specializationB = newSpecB;
	}
}

void SetArrayElement::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _array){
		_selectedOperation = 0;
		
		Numeric::Type type = _array->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeIntArray){
				_selectedOperation = &SetArrayElement::updateInt;
			}
			else if(type == Numeric::numericTypeFloatArray){
				_selectedOperation = &SetArrayElement::updateFloat;
			}
            /*
			else if(type == Numeric::numericTypeVec3Array){
				_selectedOperation = &SetArrayElement::updateVec3;
			}
			else if(type == Numeric::numericTypeCol4Array){
				_selectedOperation = &SetArrayElement::updateCol4;
			}
			else if(type == Numeric::numericTypeMatrix44Array){
				_selectedOperation = &SetArrayElement::updateMatrix44;
			}
            */
		}
	}
}

void SetArrayElement::updateInt(Numeric *array, const std::vector<int> &index, Numeric *element, Numeric *outArray, unsigned int slice){
	std::vector<int> values = array->intValuesSlice(slice);
	int valuesSize = array->sizeSlice(slice);
	for(int i = 0; i < index.size(); ++i){
		int currentIndex = index[i];
		if(currentIndex >= 0 && currentIndex < valuesSize){
			values[currentIndex] = element->intValueAtSlice(slice, i);
		}
	}

	outArray->setIntValuesSlice(slice, values);
}

void SetArrayElement::updateFloat(Numeric *array, const std::vector<int> &index, Numeric *element, Numeric *outArray, unsigned int slice){
	std::vector<float> values = array->floatValuesSlice(slice);
	int valuesSize = array->sizeSlice(slice);
	for(int i = 0; i < index.size(); ++i){
		int currentIndex = index[i];
		if(currentIndex >= 0 && currentIndex < valuesSize){
			values[currentIndex] = element->floatValueAtSlice(slice, i);
		}
	}

	outArray->setFloatValuesSlice(slice, values);
}

void SetArrayElement::updateSlice(Attribute *attribute, unsigned int slice){
	if(_selectedOperation){
		Numeric *array = _array->value();
		const std::vector<int> &index = _index->value()->intValuesSlice(slice);
		Numeric *element = _element->value();
		Numeric *outArray = _outArray->outValue();
		
		(this->*_selectedOperation)(array, index, element, outArray, slice);
	}
}

SetSimulationStep::SetSimulationStep(const std::string &name, Node *parent): 
Node(name, parent),
_selectedOperation(0){
	setSliceable(true);

	_storageKey = new StringAttribute("storageKey", this);
	_data = new NumericAttribute("data", this);
	_result = new NumericAttribute("result", this);
	
	addInputAttribute(_storageKey);
	addInputAttribute(_data);
	addOutputAttribute(_result);
	
	setAttributeAffect(_storageKey, _result);
	setAttributeAffect(_data, _result);
	
	addAttributeSpecializationLink(_data, _result);
}

void SetSimulationStep::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _data){
		_selectedOperation = 0;
		
		Numeric::Type type = _data->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeInt || type == Numeric::numericTypeIntArray){
				_selectedOperation = &SetSimulationStep::updateInt;
			}
			else if(type == Numeric::numericTypeFloat || type == Numeric::numericTypeFloatArray){
				_selectedOperation = &SetSimulationStep::updateFloat;
			}
            /*
			else if(type == Numeric::numericTypeVec3 || type == Numeric::numericTypeVec3Array){
				_selectedOperation = &SetSimulationStep::updateVec3;
			}
			else if(type == Numeric::numericTypeCol4 || type == Numeric::numericTypeCol4Array){
				_selectedOperation = &SetSimulationStep::updateCol4;
			}
			else if(type == Numeric::numericTypeMatrix44 || type == Numeric::numericTypeMatrix44Array){
				_selectedOperation = &SetSimulationStep::updateMatrix44;
			}
			else if(type == Numeric::numericTypeQuat || type == Numeric::numericTypeQuatArray){
				_selectedOperation = &SetSimulationStep::updateQuat;
			}
            */
		}
	}
}

void SetSimulationStep::updateInt(const std::string &storageKey, Numeric *data, Numeric *result, unsigned int slice){
	_globalNumericStorage[storageKey].setIntValuesSlice(slice, data->intValuesSlice(slice));
	result->setIntValuesSlice(slice, data->intValuesSlice(slice));
}

void SetSimulationStep::updateFloat(const std::string &storageKey, Numeric *data, Numeric *result, unsigned int slice){
	_globalNumericStorage[storageKey].setFloatValuesSlice(slice, data->floatValuesSlice(slice));
	result->setFloatValuesSlice(slice, data->floatValuesSlice(slice));
}
void SetSimulationStep::resizedSlices(unsigned int slices){
	_globalNumericStorage[_storageKey->value()->stringValue()].resizeSlices(slices);
}

void SetSimulationStep::updateSlice(Attribute *attribute, unsigned int slice){
	if(_selectedOperation){
		#ifdef CORAL_PARALLEL_TBB
			tbb::mutex::scoped_lock lock(_globalMutex); // block setting _globalNumericStorage from two different threads
		#endif

		(this->*_selectedOperation)(_storageKey->value()->stringValue(), _data->value(), _result->outValue(), slice);
	}
}

GetSimulationStep::GetSimulationStep(const std::string &name, Node *parent): 
Node(name, parent),
_selectedOperation(0){
	setSliceable(true);

	_storageKey = new StringAttribute("storageKey", this);
	_source = new NumericAttribute("source", this);
	_step = new NumericAttribute("step", this);
	_data = new NumericAttribute("data", this);
	
	addInputAttribute(_storageKey);
	addInputAttribute(_source);
	addInputAttribute(_step);
	addOutputAttribute(_data);
	
	setAttributeAffect(_storageKey, _data);
	setAttributeAffect(_source, _data);
	setAttributeAffect(_step, _data);
	
	std::vector<std::string> spec;
	spec.push_back("Int");
	spec.push_back("Float");
	setAttributeAllowedSpecializations(_step, spec);
	
	addAttributeSpecializationLink(_source, _data);
}

void GetSimulationStep::attributeSpecializationChanged(Attribute *attribute){
	if(attribute == _source){
		_selectedOperation = 0;
		
		Numeric::Type type = _source->outValue()->type();
		if(type != Numeric::numericTypeAny){
			if(type == Numeric::numericTypeInt || type == Numeric::numericTypeIntArray){
				_selectedOperation = &GetSimulationStep::updateInt;
			}
			else if(type == Numeric::numericTypeFloat || type == Numeric::numericTypeFloatArray){
				_selectedOperation = &GetSimulationStep::updateFloat;
			}
            /*
			else if(type == Numeric::numericTypeVec3 || type == Numeric::numericTypeVec3Array){
				_selectedOperation = &GetSimulationStep::updateVec3;
			}
			else if(type == Numeric::numericTypeCol4 || type == Numeric::numericTypeCol4Array){
				_selectedOperation = &GetSimulationStep::updateCol4;
			}
			else if(type == Numeric::numericTypeMatrix44 || type == Numeric::numericTypeMatrix44Array){
				_selectedOperation = &GetSimulationStep::updateMatrix44;
			}
			else if(type == Numeric::numericTypeQuat || type == Numeric::numericTypeQuatArray){
				_selectedOperation = &GetSimulationStep::updateQuat;
			}
            */
		}
	}
}

void GetSimulationStep::updateInt(const std::string &storageKey, int step, Numeric *source, Numeric *data, unsigned int slice){
	if(step <= 0 || _globalNumericStorage.find(storageKey) == _globalNumericStorage.end()){
		data->setIntValuesSlice(slice, source->intValuesSlice(slice));
	}
	else{
		data->setIntValuesSlice(slice, _globalNumericStorage[storageKey].intValuesSlice(slice));
	}
}

void GetSimulationStep::updateFloat(const std::string &storageKey, int step, Numeric *source, Numeric *data, unsigned int slice){
	if(step <= 0 || _globalNumericStorage.find(storageKey) == _globalNumericStorage.end()){
		data->setFloatValuesSlice(slice, source->floatValuesSlice(slice));
	}
	else{
		data->setFloatValuesSlice(slice, _globalNumericStorage[storageKey].floatValuesSlice(slice));
	}
}
void GetSimulationStep::updateSlice(Attribute *attribute, unsigned int slice){
	if(_selectedOperation){
		Numeric *step = _step->value();

		int stepVal;
		if(step->type() == Numeric::numericTypeFloat){
			stepVal = int(step->floatValueAtSlice(slice, 0));
		}
		else if(step->type() == Numeric::numericTypeInt){
			stepVal = step->intValueAtSlice(slice, 0);
		}

		(this->*_selectedOperation)(_storageKey->value()->stringValue(), stepVal, _source->value(), _data->outValue(), slice);
	}
}

