//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPrerequisitesUtil.h"
#include "BsException.h"

namespace BansheeEngine
{
	const Path Path::BLANK = Path();

	Path::Path()
		:mIsAbsolute(false)
	{ }

	Path::Path(const WString& pathStr, PathType type)
	{
		assign(pathStr, type);
	}

	Path::Path(const String& pathStr, PathType type)
	{
		assign(pathStr, type);
	}

	Path::Path(const wchar_t* pathStr, PathType type)
	{
		assign(pathStr);
	}

	Path::Path(const char* pathStr, PathType type)
	{
		assign(pathStr);
	}

	Path::Path(const Path& other)
	{
		assign(other);
	}

	Path& Path::operator= (const Path& path)
	{
		assign(path);
		return *this;
	}

	Path& Path::operator= (const WString& pathStr)
	{
		assign(pathStr);
		return *this;
	}

	Path& Path::operator= (const String& pathStr)
	{
		assign(pathStr);
		return *this;
	}

	Path& Path::operator= (const wchar_t* pathStr)
	{
		assign(pathStr);
		return *this;
	}

	Path& Path::operator= (const char* pathStr)
	{
		assign(pathStr);
		return *this;
	}

	void Path::swap(Path& path)
	{
		std::swap(mDirectories, path.mDirectories);
		std::swap(mFilename, path.mFilename);
		std::swap(mDevice, path.mDevice);
		std::swap(mNode, path.mNode);
		std::swap(mIsAbsolute, path.mIsAbsolute);
	}

	void Path::assign(const Path& path)
	{
		mDirectories = path.mDirectories;
		mFilename = path.mFilename;
		mDevice = path.mDevice;
		mNode = path.mNode;
		mIsAbsolute = path.mIsAbsolute;
	}

	void Path::assign(const WString& pathStr, PathType type)
	{
		assign(pathStr.data(), (UINT32)pathStr.length(), type);
	}

	void Path::assign(const String& pathStr, PathType type)
	{
		assign(pathStr.data(), (UINT32)pathStr.length(), type);
	}

	void Path::assign(const wchar_t* pathStr, PathType type)
	{
		assign(pathStr, (UINT32)wcslen(pathStr), type);
	}

	void Path::assign(const char* pathStr, PathType type)
	{
		assign(pathStr, (UINT32)strlen(pathStr), type);
	}

	void Path::assign(const wchar_t* pathStr, UINT32 numChars, PathType type)
	{
		switch (type)
		{
		case PathType::Windows:
			parseWindows(pathStr, numChars);
			break;
		case PathType::Unix:
			parseUnix(pathStr, numChars);
			break;
		default:
#if BS_PLATFORM == BS_PLATFORM_WIN32
			parseWindows(pathStr, numChars);
#elif BS_PLATFORM == BS_PLATFORM_OSX || BS_PLATFORM == BS_PLATFORM_LINUX
			parseUnix(pathStr, numChars);
#else
			static_assert(false, "Unsupported platform for path.");
#endif
			break;
		}
	}

	void Path::assign(const char* pathStr, UINT32 numChars, PathType type)
	{
		switch (type)
		{
		case PathType::Windows:
			parseWindows(pathStr, numChars);
			break;
		case PathType::Unix:
			parseUnix(pathStr, numChars);
			break;
		default:
#if BS_PLATFORM == BS_PLATFORM_WIN32
			parseWindows(pathStr, numChars);
#elif BS_PLATFORM == BS_PLATFORM_OSX || BS_PLATFORM == BS_PLATFORM_LINUX
			parseUnix(pathStr, numChars);
#else
			static_assert(false, "Unsupported platform for path.");
#endif
			break;
		}
	}

	WString Path::toWString(PathType type) const
	{
		switch (type)
		{
		case PathType::Windows:
			return buildWindows();
		case PathType::Unix:
			return buildUnix();
		default:
#if BS_PLATFORM == BS_PLATFORM_WIN32
			return buildWindows();
#elif BS_PLATFORM == BS_PLATFORM_OSX || BS_PLATFORM == BS_PLATFORM_LINUX
			return buildUnix();
#else
			static_assert(false, "Unsupported platform for path.");
#endif
			break;
		}
	}

	String Path::toString(PathType type) const
	{
		switch (type)
		{
		case PathType::Windows:
			return BansheeEngine::toString(buildWindows());
		case PathType::Unix:
			return BansheeEngine::toString(buildUnix());
		default:
#if BS_PLATFORM == BS_PLATFORM_WIN32
			return BansheeEngine::toString(buildWindows());
#elif BS_PLATFORM == BS_PLATFORM_OSX || BS_PLATFORM == BS_PLATFORM_LINUX
			return BansheeEngine::toString(buildUnix());
#else
			static_assert(false, "Unsupported platform for path.");
#endif
			break;
		}
	}

	Path Path::getParent() const
	{
		Path copy = *this;
		copy.makeParent();

		return copy;
	}

	Path Path::getAbsolute(const Path& base) const
	{
		Path copy = *this;
		copy.makeAbsolute(base);

		return copy;
	}

	Path Path::getRelative(const Path& base) const
	{
		Path copy = *this;
		copy.makeRelative(base);

		return copy;
	}

	Path Path::getDirectory() const
	{
		Path copy = *this;
		copy.mFilename.clear();

		return copy;
	}

	Path& Path::makeParent()
	{
		if (mFilename.empty())
		{
			if (mDirectories.empty())
			{
				if (!mIsAbsolute)
					mDirectories.push_back(L"..");
			}
			else
			{
				if (mDirectories.back() == L"..")
					mDirectories.push_back(L"..");
				else
					mDirectories.pop_back();
			}
		}
		else
		{
			mFilename.clear();
		}

		return *this;
	}

	Path& Path::makeAbsolute(const Path& base)
	{
		if (mIsAbsolute)
			return *this;

		Path absDir = base.getDirectory();
		if (base.isFile())
			absDir.pushDirectory(base.mFilename);

		for (auto& dir : mDirectories)
			absDir.pushDirectory(dir);

		absDir.setFilename(mFilename);
		*this = absDir;

		return *this;
	}

	Path& Path::makeRelative(const Path& base)
	{
		if (!base.includes(*this))
			return *this;

		mDirectories.erase(mDirectories.begin(), mDirectories.begin() + base.mDirectories.size());

		// Sometimes a directory name can be interpreted as a file and we're okay with that. Check for that
		// special case.
		if (base.isFile())
		{
			if (mDirectories.size() > 0)
				mDirectories.erase(mDirectories.begin());
			else
				mFilename = L"";
		}

		mDevice = L"";
		mNode = L"";
		mIsAbsolute = false;

		return *this;
	}

	bool Path::includes(const Path& child) const
	{
		if (mDevice != child.mDevice)
			return false;

		if (mNode != child.mNode)
			return false;

		auto iterParent = mDirectories.begin();
		auto iterChild = child.mDirectories.begin();

		for (; iterParent != mDirectories.end(); ++iterChild, ++iterParent)
		{
			if (iterChild == child.mDirectories.end())
				return false;

			if (!comparePathElem(*iterChild, *iterParent))
				return false;
		}

		if (!mFilename.empty())
		{
			if (iterChild == child.mDirectories.end())
			{
				if (child.mFilename.empty())
					return false;

				if (!comparePathElem(child.mFilename, mFilename))
					return false;
			}
			else
			{
				if (!comparePathElem(*iterChild, mFilename))
					return false;
			}			
		}

		return true;
	}

	bool Path::equals(const Path& other) const
	{
		if (mIsAbsolute != other.mIsAbsolute)
			return false;

		if (mIsAbsolute)
		{
			if (!comparePathElem(mDevice, other.mDevice))
				return false;
		}

		if (!comparePathElem(mNode, other.mNode))
			return false;

		UINT32 myNumElements = (UINT32)mDirectories.size();
		UINT32 otherNumElements = (UINT32)other.mDirectories.size();

		if (!mFilename.empty())
			myNumElements++;

		if (!other.mFilename.empty())
			otherNumElements++;

		if (myNumElements != otherNumElements)
			return false;

		if(myNumElements > 0)
		{
			auto iterMe = mDirectories.begin();
			auto iterOther = other.mDirectories.begin();

			for(UINT32 i = 0; i < (myNumElements - 1); i++, ++iterMe, ++iterOther)
			{
				if (!comparePathElem(*iterMe, *iterOther))
					return false;
			}

			if (!mFilename.empty())
			{
				if (!other.mFilename.empty())
				{
					if (!comparePathElem(mFilename, other.mFilename))
						return false;
				}
				else
				{
					if (!comparePathElem(mFilename, *iterOther))
						return false;
				}
			}
			else
			{
				if (!other.mFilename.empty())
				{
					if (!comparePathElem(*iterMe, other.mFilename))
						return false;
				}
				else
				{
					if (!comparePathElem(*iterMe, *iterOther))
						return false;
				}
			}
		}

		return true;
	}

	Path& Path::append(const Path& path)
	{
		if (!mFilename.empty())
			pushDirectory(mFilename);

		for (auto& dir : path.mDirectories)
			pushDirectory(dir);

		mFilename = path.mFilename;

		return *this;
	}

	void Path::setBasename(const WString& basename)
	{
		mFilename = basename + getWExtension();
	}

	void Path::setBasename(const String& basename)
	{
		mFilename = BansheeEngine::toWString(basename) + getWExtension();
	}

	void Path::setExtension(const WString& extension)
	{
		WStringStream stream;
		stream << getWFilename(false);
		stream << extension;

		mFilename = stream.str();
	}

	void Path::setExtension(const String& extension)
	{
		setExtension(BansheeEngine::toWString(extension));
	}

	WString Path::getWFilename(bool extension) const
	{
		if (extension)
			return mFilename;
		else
		{
			WString::size_type pos = mFilename.rfind(L'.');
			if (pos != WString::npos)
				return mFilename.substr(0, pos);
			else
				return mFilename;
		}
	}

	String Path::getFilename(bool extension) const
	{
		return BansheeEngine::toString(getWFilename(extension));
	}

	WString Path::getWExtension() const
	{
		WString::size_type pos = mFilename.rfind(L'.');
		if (pos != WString::npos)
			return mFilename.substr(pos);
		else
			return WString();
	}

	String Path::getExtension() const
	{
		return BansheeEngine::toString(getWExtension());
	}

	const WString& Path::getWDirectory(UINT32 idx) const
	{
		if (idx >= (UINT32)mDirectories.size())
		{
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + BansheeEngine::toString(idx) + 
				". Valid range: [0, " + BansheeEngine::toString((UINT32)mDirectories.size() - 1) + "]");
		}

		return mDirectories[idx];
	}

	String Path::getDirectory(UINT32 idx) const
	{
		return BansheeEngine::toString(getWDirectory(idx));
	}

	WString Path::getWTail(PathType type) const
	{
		if (isFile())
			return mFilename;
		else if (mDirectories.size() > 0)
			return mDirectories.back();
		else
			return StringUtil::WBLANK;
	}

	String Path::getTail(PathType type) const
	{
		return BansheeEngine::toString(getWTail(type));
	}

	void Path::clear()
	{
		mDirectories.clear();
		mDevice.clear();
		mFilename.clear();
		mNode.clear();
		mIsAbsolute = false;
	}

	void Path::throwInvalidPathException(const WString& path) const
	{
		BS_EXCEPT(InvalidParametersException, "Incorrectly formatted path provided: " + BansheeEngine::toString(path));
	}

	void Path::throwInvalidPathException(const String& path) const
	{
		BS_EXCEPT(InvalidParametersException, "Incorrectly formatted path provided: " + path);
	}

	WString Path::buildWindows() const
	{
		WStringStream result;
		if (!mNode.empty())
		{
			result << L"\\\\";
			result << mNode;
			result << L"\\";
		}
		else if (!mDevice.empty())
		{
			result << mDevice;
			result << L":\\";
		}
		else if (mIsAbsolute)
		{
			result << L"\\";
		}

		for (auto& dir : mDirectories)
		{
			result << dir;
			result << L"\\";
		}

		result << mFilename;
		return result.str();
	}

	WString Path::buildUnix() const
	{
		WStringStream result;
		auto dirIter = mDirectories.begin();

		if (!mDevice.empty())
		{
			result << L"/";
			result << mDevice;
			result << L":/";
		}
		else if (mIsAbsolute)
		{
			if (dirIter != mDirectories.end() && *dirIter == L"~")
			{
				result << L"~";
				dirIter++;
			}

			result << L"/";
		}

		for (; dirIter != mDirectories.end(); ++dirIter)
		{
			result << *dirIter;
			result << L"/";
		}

		result << mFilename;
		return result.str();
	}

	Path Path::operator+ (const Path& rhs) const
	{
		return Path::combine(*this, rhs);
	}

	Path& Path::operator+= (const Path& rhs)
	{
		return append(rhs);
	}

	bool Path::comparePathElem(const WString& left, const WString& right)
	{
		if (left.size() != right.size())
			return false;

		// TODO: Case sensitive/insensitive file path actually depends on used file-system but I'm not gonna check that
		for (UINT32 i = 0; i < (UINT32)left.size(); i++)
		{
			if (tolower(left[i]) != tolower(right[i]))
				return false;
		}

		return true;
	}

	Path Path::combine(const Path& left, const Path& right)
	{
		Path output = left;
		return output.append(right);
	}

	void Path::pushDirectory(const WString& dir)
	{
		if (!dir.empty() && dir != L".")
		{
			if (dir == L"..")
			{
				if (!mDirectories.empty() && mDirectories.back() != L"..")
					mDirectories.pop_back();
				else
					mDirectories.push_back(dir);
			}
			else
				mDirectories.push_back(dir);
		}
	}

	void Path::pushDirectory(const String& dir)
	{
		pushDirectory(BansheeEngine::toWString(dir));
	}
}
