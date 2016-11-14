/**
 * Headers
 */
#pragma once

#include <string>
#include <vector>

/**
 * class definition
 */
class CPageRepPolicy
{
public:
	/* Construction/Destructor */
	CPageRepPolicy(char* pFileName);
	~CPageRepPolicy();
	
	/* Initial and only routine for outside to call */
	int Start();

private:
	int ReadFile();		/* Read from file */

	void InitFrames();	/* Initialize frames with -1 */

	void Display();		/* Display the frame result */

	int FIFO();		/* FIFO policy implementation */

	int Optimal();		/* Optimal policy implementation */

	int LRU();		/* Least Recently used policy implementation */

	bool SetPolicy(std::string csType);	/* Set policy type, convert from string */

	inline void SetType(unsigned int nType)	/* Set enum based policy type */
	{
		m_nType = nType;
	}
	inline unsigned int GetTYpe() const	/* Get enum based policy type */
	{
		return m_nType;
	}
	inline bool IsFIFO() const		/* If policy is FIFO */
	{
		return m_nType == TYPE_FIFO ? true : false;
	}
	inline bool IsOptimal() const		/* If policy is Optimal */
	{
		return m_nType == TYPE_OPT ? true : false;
	}
	inline bool IsLRU() const		/* If policy is Least Recently Used */
	{
		return m_nType == TYPE_LRU ? true : false;
	}
	
	inline void SetPages(unsigned int nPages)	/* Set number of pages */
	{
		m_nPages = nPages;
	}
	inline unsigned int GetPages() const		/* Get number of pages */
	{
		return m_nPages;
	}
	
	inline void SetFrames(unsigned int nFrames)	/* Set number of frames */
	{
		m_nFrames = nFrames;
	}
	inline unsigned int GetFrames() const		/* Get number frames */
	{
		return m_nFrames;
	}
	
private:
	unsigned int m_nPages;		/* Number of pages */
	unsigned int m_nType;		/* Type of policy */
	unsigned int m_nFrames;		/* Number of frames */
	int* m_pFrame;			/* Frames buffer */
	std::vector<int> m_cPages;	/* Pages buffer */
	
	char* m_pFileName;		/* File name of file to read data from */
};
