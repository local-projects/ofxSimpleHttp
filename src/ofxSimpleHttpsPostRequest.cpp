//
//  PostRequestMaker.cpp
//  ofxHttpFormExample
//
//  Created by Cameron Erdogan on 2/13/20.
//

#include "ofxSimpleHttpsPostRequest.h"

PostRequestResponse ofxSimpleHttpsPostRequest::makePostRequest(string url, string body, map<string,string> headers) {
    try
    {
        // prepare session
        URI uri(url);
        
        const Poco::Net::Context::Ptr context = new Poco::Net::Context(
            Poco::Net::Context::CLIENT_USE, "", "", "",
            Poco::Net::Context::VERIFY_NONE, 9, false,
            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        
        HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
        
        // prepare path
        string path(uri.getPathAndQuery());
        if (path.empty()) path = "/";
        
        // send request
        HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
        req.setContentType("application/x-www-form-urlencoded");
        
        // Set headers here
        for(map<string,string>::iterator it = headers.begin();
            it != headers.end(); it++) {
            req.set(it->first, it->second);
        }
        
        // Set the request body
        req.setContentLength( body.length() );
        
        // sends request, returns open stream
        std::ostream& os = session.sendRequest(req);
        os << body;  // sends the body
        //req.write(std::cout); // print out request
        
        // get response
        HTTPResponse res;
        cout << res.getStatus() << " " << res.getReason() << endl;
        
        istream &is = session.receiveResponse(res);
        stringstream ss;
        StreamCopier::copyStream(is, ss);
        
        PostRequestResponse response;
        response.status = res.getStatus();
        response.responseString = ss.str();
        response.successful = true;
        
        return response;
    }
    catch (Exception &ex)
    {
        cerr << "Post Request Error: " << ex.displayText() << endl;
        
        PostRequestResponse response;
        return response;
    }
}
