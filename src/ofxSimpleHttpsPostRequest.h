//
//  SimpleHTTPSPostRequest.hpp
//  ofxHttpFormExample
//
//  Created by Cameron Erdogan on 2/13/20.
//
#pragma once

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>

#include "ofMain.h"

using namespace Poco::Net;
using namespace Poco;


struct PostRequestResponse{
    Poco::Net::HTTPResponse::HTTPStatus status; //this is going to be some whack number if the request fails
    string responseString;
    bool successful = false;
};

class ofxSimpleHttpsPostRequest{
    
public:
    PostRequestResponse makePostRequest(string url, string body, map<string,string> headers);
};

