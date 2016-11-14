/**
 * Header files
 */
#include <fstream>

#include "support.h"
#include "log.h"
#include "PageRepPolicy.h"

/**
 * Constructor
 */
CPageRepPolicy::CPageRepPolicy(char* pFileName)
{
	m_pFrame = NULL;
	m_pFileName = pFileName;
}

/**
 * Destructor
 */
CPageRepPolicy::~CPageRepPolicy()
{
	if (m_pFrame)
		delete[] m_pFrame;
}

/**
 * InitFrames
 * 
 * Initialize the memory for frames
 * If memory is already initialized, just delete it
 */
void CPageRepPolicy::InitFrames()
{
	debug_log("Entering %s ...", __FUNCTION__);
	try {
		if (m_pFrame != NULL)
			delete[] m_pFrame;		/* delete existing memory */
		m_pFrame = new int[GetFrames()];	/* Create new memory for frame buffer */
		for (int nFrame = 0; nFrame < GetFrames(); ++ nFrame)
			m_pFrame[nFrame] = -1;		/* Initialize all frames with -1 */
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown Exception...");
	}
	debug_log("Exiting %s ...", __FUNCTION__);
}

/**
 * Start
 * 
 * Read the file
 * Initialize frame buffer
 * Check policy Type and apply the respective algorithm
 */
int CPageRepPolicy::Start()
{
	debug_log("Entering %s ...", __FUNCTION__);
	int nRes = -1;
	try {
		nRes = ReadFile();		/* Read the input file */
		InitFrames();			/* Initialize the frame buffer */
		if (IsFIFO())			/* If policy is FIFO */
			nRes = FIFO();		/* FIFO algorithm */
		else if (IsOptimal())		/* If policy is Optimal */
			nRes = Optimal();	/* Optimal algorithm */
		else if (IsLRU())		/* If policy is LRU */
			nRes = LRU();		/* Least Recently Used algorithm */
		else
			perr_printf("Invalid policy...");	/* Invalid algorithm type */
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown Exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/**
 * ReadFile
 * 
 * Open the file, read a line
 * read the policy type, frame size and pages
 */
int CPageRepPolicy::ReadFile()
{
	debug_log("Entering %s ...", __FUNCTION__);
	if (m_pFileName == NULL) {
		err_printf("Invalid parameter, filename not found");
		return -1;
	}
	std::ifstream inputFile(m_pFileName);	/* open the file for reading */
	std::string csLine;
	std::string csItem;
	int nRes = 0;
	try {
		while (std::getline(inputFile, csLine)) {	/* read one line from the file */
			debug_log(csLine.c_str());		/* debugging log to display the line */
			int nIndex = csLine.find(",");		/* find , as separator */
			csItem = csLine.substr(0, nIndex);	/* separate the job numbfer */
			debug_log("Algo # %s", csItem.c_str());	/* debugging log to display the job number */
			if (!SetPolicy(csItem)) {
				nRes = -1;
				break;
			}
			nIndex ++;
			int nNextIndex = csLine.find(",", nIndex);	/* get next item in line */
			csItem = csLine.substr(nIndex, nNextIndex - nIndex);	/* get frame buffer size */
			unsigned int nFrames = atoi(csItem.c_str());	/* Convert to integer */
			SetFrames(nFrames);				/* Set frame size */
			do {
				nIndex = ++ nNextIndex;
				nNextIndex = csLine.find(",", nIndex);	/* find next available ',' */
				csItem = csLine.substr(nIndex, nNextIndex - nIndex);	/* get next item */
				int nPage = atoi(csItem.c_str());	/* convert the next item (page) to integer value */
				m_cPages.push_back(nPage);		/* push it to vector */
			} while (nNextIndex != -1);			/* unless we have items */
			SetPages(m_cPages.size());			/* set number of pages */
		}
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown Exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/**
 * SetPolicy
 * 
 * Policy item is read, parse it and set the appropriate policy enum type
 */
bool CPageRepPolicy::SetPolicy(std::string csType)
{
	debug_log("Entering %s ...", __FUNCTION__);
	bool bRes = false;
	try {
		if (csType.length() > 1 && csType.length() < 1)		/* Check boundry conditions for policy */
			perr_printf("Couldn't set the policy type");
		else {
			bRes = true;
			if (csType.compare("F") == 0)			/* This is FIFO policy */
				SetType(TYPE_FIFO);
			else if (csType.compare("O") == 0)		/* This is Optimal policy */
				SetType(TYPE_OPT);
			else if (csType.compare("L") == 0)		/* This is Least recently used policy */
				SetType(TYPE_LRU);
			else {
				perr_printf("Couldn't set the policy type");	/* unknown policy */
				bRes = false;
			}
		}
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown Exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, bRes, bRes);
	return bRes;
}

/**
 * Display
 * 
 * Display the current frame buffer
 */
void CPageRepPolicy::Display()
{
	try {
		log_message_no_endl("<");					/* Initial output */
		for (int nFrame = 0; nFrame < GetFrames(); ++ nFrame) {		/* traverse all frames */
			if (m_pFrame[nFrame] != -1)				/* if valid frames */
				log_message_no_endl("%d", m_pFrame[nFrame]);	/* print current frame */
			else							/* otherwise, */
				log_message_no_endl(" ");			/* just print space */
			if (nFrame < GetFrames() - 1)				/* If this isn't last item in frame buffer */
				log_message_no_endl(", ");			/* print , */
			else							/* otherwise */
				log_message(">");				/* print end delimiter */
		}
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown Exception...");
	}
}

/**
 * FIFO
 * 
 * Get all the pages one by one
 * Check the available frame
 * Update page fault and frame buffer
 */
int CPageRepPolicy::FIFO()
{
	debug_log("Entering %s ...", __FUNCTION__);
	int nRes = 0;
	try {
		int nNextFrame = 0;
		int nCount = 0;
		log_message("For FIFO");
		for (int nPage = 0; nPage < GetPages(); ++ nPage) {			/* For each page */
			log_message_no_endl("%d: ", m_cPages[nPage]);			/* Log the current page number */
			bool bAvailable = false;
			for (int nFrame = 0; nFrame < GetFrames(); ++ nFrame)		/* for each frame in frame buffer */
				if (m_pFrame[nFrame] == m_cPages[nPage])		/* if current frame is already the same page */
					bAvailable = true;				/* this page is already loaded */
				if (!bAvailable) {
					m_pFrame[nNextFrame] = m_cPages[nPage];		/* if page isn't loaded, load it in frame buffer */
					nNextFrame = (nNextFrame + 1) % GetFrames();	/* Set next frame */
					++ nCount;					/* Set page fault counter */
				}
				Display();						/* display frame buffer */
		}
		log_message("Total number of faults: %d", nCount);			/* display page fault count */
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown Exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/**
 * Optimal
 * 
 * Load the pages in frame buffer based on counter array
 * in order to check the future which page is requested more
 * then the other page
 */
int CPageRepPolicy::Optimal()
{
	debug_log("Entering %s ...", __FUNCTION__);
	int nRes = 0;
	try {
		bool bFound = false;
		int nMax = 0;
		int nCount = 0;
		int nCurrentFrame = 0;
		int nNewPage = 0;
		const int nFrameSize = GetPages();
		int pCounter[nFrameSize];
		memset(pCounter, 0, nFrameSize);
		log_message("For Optimal");
		for (int nPage = 0; nPage < GetPages();++ nPage) {			/* for each page */
			bool bFlag = false;
			log_message_no_endl("%d: ", m_cPages[nPage]);			/* display page number */
			for (int nFrame = 0; nFrame < GetFrames(); ++ nFrame) {		/* check frames */
				if (m_cPages[nPage] == m_pFrame[nFrame]) {		/* if they already exists */
					bFlag = true;					/* set available to true */
					break;
				}
			}
			if ((!bFlag) && (nCurrentFrame < GetFrames())) {		/* if page isn't in frame, and space available */
				++ nCount;						/* page fault */
				m_pFrame[nCurrentFrame] = m_cPages[nPage];		/* set the page in frame buffer */
				++ nCurrentFrame;
			}
			else if ((!bFlag) && (nCurrentFrame == GetFrames())) {		/* if page isn't in frame and no space */
				++ nCount;						/* page fault */
				for (int nFrameIndex = 0; nFrameIndex < GetFrames(); ++ nFrameIndex) {	/* for each frame */
					for (int nCurrentPage = nFrameIndex; nCurrentPage < GetPages(); ++ nCurrentPage) {	/* for each page loaded in frame */
						if (m_pFrame[nFrameIndex] != m_cPages[nCurrentPage])	/* if frame isn't available in pages */
							pCounter[nFrameIndex] ++;	/* update the counter frame index */
						else
							break;				/* we are done */
					}
				}
				int nMaxUsed = 0;
				for(int nFrm = 0; nFrm < GetFrames(); ++ nFrm) {	/* for each frame */
					if (pCounter[nFrm] > nMaxUsed) {		/* check the counter frame index for next page */
						nMaxUsed = pCounter[nFrm];		/* Set the max used frame in future from counter */
						nNewPage = nFrm;			/* set the new page to be loaded */
					}
				}
				m_pFrame[nNewPage]= m_cPages[nPage];			/* load the page in frame buffer */
			}
			Display();							/* display the frame buffer */
		}
		log_message("Total number of faults: %d", nCount);			/* display the page fault */
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown Exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/**
 * LRU
 * 
 * Load the pages in frame buffer based on counter array
 * in order to check the currently used pages
 */
int CPageRepPolicy::LRU()
{
	debug_log("Entering %s ...", __FUNCTION__);
	int nRes = 0;
	try {
		const int nFrameSize = GetPages();
		int pCounter[nFrameSize];
		memset(pCounter, 0, nFrameSize);
		unsigned int nLeast = 0;
		unsigned int nCount = 0;
		unsigned int nNewPage = 0;
		unsigned int nCurrentFrame = 0;
		log_message("For LRU");
		for (int nPage = 0; nPage < GetPages(); ++ nPage) {			/* for each pages */
			bool bFlag = false;
			log_message_no_endl("%d: ", m_cPages[nPage]);			/* display the page number */
			for (int nFrame = 0; nFrame < GetFrames(); ++ nFrame) {		/* for each frame in buffer */
				if (m_cPages[nPage] == m_pFrame[nFrame]) {		/* check if page is available in frame buffer */
					bFlag = true;
					break;
				}
			}
			if ((!bFlag) && (nLeast < GetFrames())) {			/* if page isn't in frame, and space available */
				++ nCount;						/* page fault */
				m_pFrame[nLeast ++] = m_cPages[nPage];			/* set page in frame buffer */
			}
			else if ((!bFlag) && (nLeast == GetFrames())) {			/* if page isn't in frame, and no space */
				++ nCount;						/* page fault */
				for (int nFrame = 0; nFrame < GetFrames(); ++ nFrame) {	/* for each frame */
					nCurrentFrame = m_pFrame[nFrame];		/* get current frame */
					for (int nCurrentPage = nPage; nCurrentPage > 0; -- nCurrentPage) {	/* for page loaded in frames so far */
						if (nCurrentFrame != m_cPages[nCurrentPage])	/* if it hasn't been loaded */
							pCounter[nFrame] ++;		/* set the counter */
						else
							break;				/* we are done */
					}
				}
				int nMaxUsed = 0;
				for (int nFrame = 0; nFrame < GetFrames(); ++ nFrame) {	/* for each frame */
					if (pCounter[nFrame] > nMaxUsed) {		/* if this frame is least used */
						nMaxUsed = pCounter[nFrame];		/* update the max used from counter for this frame */
						nNewPage = nFrame;			/* set the frame for page fault */
					}
				}
				m_pFrame[nNewPage] = m_cPages[nPage];			/* update the frame with page */
			}

			Display();							/* display the frames */
		}
		log_message("Total number of faults: %d", nCount);			/* display the page faults */
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown Exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}
