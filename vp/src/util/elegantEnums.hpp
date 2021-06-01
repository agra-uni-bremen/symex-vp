/*
 * Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * elegantEnums.hpp
 *
 *  Created on: 14 Mar 2019
 *      Author: dwd
 */

#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define STRING_REMOVE_CHAR(str, ch) str.erase(std::remove(str.begin(), str.end(), ch), str.end())

std::vector<std::string> splitString(std::string str, char sep = ',');

#define DECLARE_ENUM_WITH_TYPE(E, T, ...)                                         \
	enum class E : T { __VA_ARGS__ };                                             \
	typedef T E##_underlying_type;                                                \
	static std::map<T, std::string> E##MapName(generateEnumMap<T>(#__VA_ARGS__)); \
	std::ostream &operator<<(std::ostream &os, E enumTmp);                        \
	size_t operator*(E enumTmp);                                                  \
	std::string operator~(E enumTmp);                                             \
	std::string operator+(std::string &&str, E enumTmp);                          \
	std::string operator+(E enumTmp, std::string &&str);                          \
	std::string &operator+=(std::string &str, E enumTmp);                         \
	E operator++(E &enumTmp);                                                     \
	bool valid##E(T value);                                                       \
	bool operator==(E left, E right);                                             \
	template<typename Other>                                                      \
	bool operator<(E left, Other right) {                                         \
		return static_cast<T>(left) < right;                                      \
	}

#define DECLARE_ENUM(E, ...) DECLARE_ENUM_WITH_TYPE(E, uint16_t, __VA_ARGS__)

#define IMPL_ENUM_WITH_TYPE(E, T)                                            \
	std::ostream &operator<<(std::ostream &os, E enumTmp) {                  \
		os << E##MapName[static_cast<T>(enumTmp)];                           \
		return os;                                                           \
	}                                                                        \
	size_t operator*(E enumTmp) {                                            \
		(void)enumTmp;                                                       \
		return E##MapName.size();                                            \
	}                                                                        \
	std::string operator~(E enumTmp) {                                       \
		return E##MapName[static_cast<T>(enumTmp)];                          \
	}                                                                        \
	std::string operator+(std::string &&str, E enumTmp) {                    \
		return str + E##MapName[static_cast<T>(enumTmp)];                    \
	}                                                                        \
	std::string operator+(E enumTmp, std::string &&str) {                    \
		return E##MapName[static_cast<T>(enumTmp)] + str;                    \
	}                                                                        \
	std::string &operator+=(std::string &str, E enumTmp) {                   \
		str += E##MapName[static_cast<T>(enumTmp)];                          \
		return str;                                                          \
	}                                                                        \
	E operator++(E &enumTmp) {                                               \
		auto iter = E##MapName.find(static_cast<T>(enumTmp));                \
		if (iter == E##MapName.end() || std::next(iter) == E##MapName.end()) \
			iter = E##MapName.begin();                                       \
		else {                                                               \
			++iter;                                                          \
		}                                                                    \
		enumTmp = static_cast<E>(iter->first);                               \
		return enumTmp;                                                      \
	}                                                                        \
	bool valid##E(T value) {                                                 \
		return (E##MapName.find(value) != E##MapName.end());                 \
	}                                                                        \
	bool operator==(E left, E right) {                                       \
		return static_cast<T>(left) == static_cast<T>(right);                \
	}


#define IMPL_ENUM(E) IMPL_ENUM_WITH_TYPE(E, int32_t)

template <typename T>
std::map<T, std::string> generateEnumMap(std::string strMap) {
	STRING_REMOVE_CHAR(strMap, ' ');
	STRING_REMOVE_CHAR(strMap, '(');

	std::vector<std::string> enumTokens(splitString(strMap));
	std::map<T, std::string> retMap;
	T inxMap;

	inxMap = 0;
	for (auto iter = enumTokens.begin(); iter != enumTokens.end(); ++iter) {
		// Token: [EnumName | EnumName=EnumValue]
		std::string enumName;
		//T enumValue;
		if (iter->find('=') == std::string::npos) {
			enumName = *iter;
		} else {
			std::vector<std::string> enumNameValue(splitString(*iter, '='));
			enumName = enumNameValue[0];
			// inxMap = static_cast<T>(enumNameValue[1]);
			if (std::is_unsigned<T>::value) {
				inxMap = static_cast<T>(std::stoull(enumNameValue[1], 0, 0));
			} else {
				inxMap = static_cast<T>(std::stoll(enumNameValue[1], 0, 0));
			}
		}
		retMap[inxMap++] = enumName;
	}

	return retMap;
}
