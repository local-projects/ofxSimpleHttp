//
//  ofxDownloadCentral.cpp
//  emptyExample
//
//  Created by Oriol Ferrer Mesià on 14/03/14.
//
//


#include "ofxDownloadCentral.h"
#include "ofEvents.h"
#include "ofEventUtils.h"
#include "Poco/LocalDateTime.h"
#include "Poco/DateTimeFormatter.h"

ofxDownloadCentral::ofxDownloadCentral(){

	verbose = true;
	busy = false;
	speedLimit = 0.0f;
	onlySkipDownloadIfChecksumMatches = true;
	maxURLsToList = 4;
	idleTimeAfterDownload = 0.0f;
	maxConcurrentDownloads = 1;
	timeOut = -1.0f;
	failedJobsSoFar = failedJobsStartNumber = 0;
}

ofxDownloadCentral::~ofxDownloadCentral(){
	cancelAllDownloads();
}


void ofxDownloadCentral::setMaxConcurrentDownloads(int numConcurrentDownloads){
	maxConcurrentDownloads = ofClamp(numConcurrentDownloads, 1, INT_MAX);
}

void ofxDownloadCentral::setCopyBufferSize(float bufferInKb){
	copyBufferSize = bufferInKb;
}

void ofxDownloadCentral::setChecksumType(ofxChecksum::Type type){
	checksumType = type;
}


void ofxDownloadCentral::startDownloading(){
	if (!busy){
		downloadStartJobsNumber = downloaders.size();
		failedJobsStartNumber = failedJobsSoFar;
		startQueue();
	}
}


void ofxDownloadCentral::setMaxURLsToList(int max){
	maxURLsToList = max;
}


void ofxDownloadCentral::setVerbose(bool b){
	verbose = b;
}

void ofxDownloadCentral::setIdleTimeAfterEachDownload(float seconds){
	idleTimeAfterDownload = seconds;
}



void ofxDownloadCentral::setNeedsChecksumMatchToSkipDownload(bool needs){
	onlySkipDownloadIfChecksumMatches = needs;
}

void ofxDownloadCentral::startQueue(){

	for(int i = activeDownloaders.size(); i < maxConcurrentDownloads; i++){
		if(downloaders.size() > 0){
			busy = true;
			ofxBatchDownloader * bd = downloaders[0];
			bd->startDownloading();
			activeDownloaders.push_back(bd);
			downloaders.erase(downloaders.begin());
		}
	}
}


void ofxDownloadCentral::update(){

	if (busy){

		for(int i = activeDownloaders.size() - 1; i >= 0; i--){

			ofxBatchDownloader * bd = activeDownloaders[i];
			bd->update();

			if (!bd->isBusy()){ //this one is over! start the next one!
				if(avgSpeed[i] <= 0.001f){
					avgSpeed[i] = bd->getAverageSpeed();
				}else{
					avgSpeed[i] = avgSpeed[i] * 0.75 + 0.25 * bd->getAverageSpeed();
				}

				downloadedSoFar += bd->getDownloadedBytesSoFar();
				if(bd->getNumFailedUrls() > 0) failedJobsSoFar ++;
				activeDownloaders.erase(activeDownloaders.begin() + i);
				delete bd;
			}
		}
		if(activeDownloaders.size() < maxConcurrentDownloads){
			startQueue();
		}
		if(activeDownloaders.size() == 0){
			busy = false;
		}
	}
}


void ofxDownloadCentral::cancelCurrentDownload(){
	if (busy){
		for(int i = activeDownloaders.size() - 1; i >= 0; i--){
			ofxBatchDownloader * bd = activeDownloaders[i];
			bd->cancelBatch(true);
		}
	}
}


void ofxDownloadCentral::cancelAllDownloads(){
	if (busy){
		cancelCurrentDownload(); //active downloads get stopped
		while(downloaders.size() > 0){
			delete downloaders[0];
			downloaders.erase(downloaders.begin());
		}
	}
}


bool ofxDownloadCentral::isBusy(){
	return busy;
}


int ofxDownloadCentral::getNumPendingDownloads(){

	int c = 0;
	for(int i = 0; i < downloaders.size(); i++){
		std::vector<std::string> pending = downloaders[i]->pendingURLs();
		c+= pending.size();
	}
	return c;
}


int ofxDownloadCentral::getNumActiveDownloads(){
	return activeDownloaders.size();
}


void ofxDownloadCentral::setMaxRetries(int maxRet){
	maxRetries = std::max(0, maxRet);
}

void ofxDownloadCentral::setSpeedLimit(float KB_per_sec){
	speedLimit = KB_per_sec;
}


void ofxDownloadCentral::setTimeOut(float timeoutSecs){
	timeOut = timeoutSecs;
}


void ofxDownloadCentral::setProxyConfiguration(const ofxSimpleHttp::ProxyConfig & c){
	proxyConfig = c;
}

void ofxDownloadCentral::setCredentials(const std::string& user, const std::string& password){
	credentials.first = user;
	credentials.second = password;
}

void ofxDownloadCentral::setOAuthToken(const std::string token){
    oauthToken = token;
}

std::string ofxDownloadCentral::getDrawableInfo(bool drawAllPending, bool detailed){

	int total = 0;
	for(int i = 0; i < activeDownloaders.size(); i++){
		total += activeDownloaders[i]->getNumSuppliedUrls();
	}

	for(int i = 0; i < downloaders.size(); i++){
		total += downloaders[i]->getNumSuppliedUrls();
	}

	std::string httpDownloadersStatus;
	std::string pendingDownloadsList;
	int numQueuedJobs = downloaders.size() + activeDownloaders.size();

	std::vector<std::string> allURLs;
	std::vector<std::string> allPending;
	bool reachedListMaxLen = false;
	int c = 0; //count how many pending urls have we printed
	bool printingList = false; //are we printing a pending list?

	if(activeDownloaders.size() > 0){

		if(!detailed) httpDownloadersStatus += "\n//// Active Downloaders (" + ofToString(activeDownloaders.size()) + ") Status ///////////////////////////////";

		for(int i = 0; i < activeDownloaders.size(); i++){
			ofxBatchDownloader * bd = activeDownloaders[i];

			if(!detailed){
				char aux[5];
				sprintf(aux, "%02d", i);
				httpDownloadersStatus += "\n//   (" + std::string(aux) + ") " + bd->getMinimalDrawableString();
				if(i == activeDownloaders.size() - 1) httpDownloadersStatus += "\n\n"; //last line more spce
			}else{
				httpDownloadersStatus += bd->getDrawableString();
			}
			total -= bd->getNumSuppliedUrls() - bd->pendingDownloads();
		}

		if(drawAllPending){
			printingList = true;
			pendingDownloadsList += "//// Remaining Downloads (" + ofToString(total) + ") /////////////////////////////////////\n";
			for(int i = 0; i < activeDownloaders.size(); i++){
				std::vector<std::string> pending = activeDownloaders[i]->pendingURLs();
				for(int j = 0; j < pending.size(); j++){
					allPending.push_back(pending[j]);
					if (c < maxURLsToList){
						pendingDownloadsList += "//   " + pending[j] + "\n";
					}
					if (c == maxURLsToList ){
						pendingDownloadsList += "//   ...\n";
						reachedListMaxLen = true;
					}
					c++;
				}
				if (reachedListMaxLen) break;
			}
		}
	}

	if(drawAllPending){
		if(downloaders.size() > 0 && !reachedListMaxLen){
			for(int i = 0; i < downloaders.size(); i++){
				std::vector<std::string> pending = downloaders[i]->pendingURLs();
				for(int j = 0; j < pending.size(); j++){
					allPending.push_back(pending[j]);
					if (c < maxURLsToList){
						pendingDownloadsList += "//   " + pending[j] + "\n";
					}
					if (c == maxURLsToList ){
						pendingDownloadsList += "//   ...\n";
						reachedListMaxLen = true;
					}
					c++;
				}
				if (reachedListMaxLen ) break;
			}
		}
	}
	if(printingList) pendingDownloadsList += "//////////////////////////////////////////////////////////////////";

	float elapsedTime = ofGetElapsedTimef() - downloadStartTime;
	float timeLeft = 0.0f;
	int numProcessed = downloadStartJobsNumber - numQueuedJobs;
	int numFailed = failedJobsSoFar - failedJobsStartNumber;
	std::string estFinish;
	if (numProcessed > 0){
		timeLeft = numQueuedJobs * elapsedTime / float(numProcessed);
		Poco::Timespan timeToAdd;
		timeToAdd.assign(timeLeft, 0);
		std::string timeFormat = "%H:%M on %w %e, %b %Y";
		Poco::LocalDateTime now;
		now += timeToAdd;
		estFinish = Poco::DateTimeFormatter::format(now, timeFormat);
	}
	float avg = 0;
	for(int i = 0; i < activeDownloaders.size(); i++){
		avg += avgSpeed[i];
	}

	std::string spa = "     ";
	std::string header = "//// ofxDownloadCentral queued Jobs: " + ofToString(numQueuedJobs) + " ///////////////////////////\n"
	"//// Jobs executed:        " + spa + ofToString(numProcessed) + "\n" +
	"//// Jobs failed:          " + spa + ofToString(numFailed) + "\n" +
//	"//// Total Downloads Left: " + spa + ofToString(total) + "\n" +
	"//// Elapsed Time:         " + spa + ofxSimpleHttp::secondsToHumanReadable(elapsedTime, 1) + "\n" +
	"//// Estimated Time Left:  " + spa + ofxSimpleHttp::secondsToHumanReadable(timeLeft, 1) + "\n"
	"//// Estimated Completion: " + spa + estFinish + "\n" +
	"//// Avg Download Speed:   " + spa + ofxSimpleHttp::bytesToHumanReadable(avg, 1) + "/sec\n" +
	"//// Downloaded So Far:    " + spa + ofxSimpleHttp::bytesToHumanReadable(downloadedSoFar, 1) +
	"\n";

	return header + httpDownloadersStatus + pendingDownloadsList;
}


void ofxDownloadCentral::draw(float x, float y, bool drawAllPending, bool detailedDownloadInfo){
	if (busy){
		std::string aux = getDrawableInfo(drawAllPending, detailedDownloadInfo);
		ofDrawBitmapString(aux, x, y);
	}
}
