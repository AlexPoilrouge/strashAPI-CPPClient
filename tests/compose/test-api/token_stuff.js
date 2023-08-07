const jwt= require("jsonwebtoken");

const fs = require('fs');
const path= require('path');


let hereLog= (...args) => {console.log("[token_stuff]", ...args);};


var the_key= null

const VERIFY_OPTIONS= {algorithm: ["RS256"]}

const TOKEN_PRIVATE_KEY_FILE= "/app/keys/public.key"

function prepareKeyFromKeyFile(){
    the_key= fs.readFileSync(path.resolve(__dirname, TOKEN_PRIVATE_KEY_FILE));
}


function verifyTokenFromReq(req, res, next){
    // hereLog("let's verify some tokenssss")
    const token = req.body.token || req.query.token || req.headers["x-access-token"];
    if(!Boolean(token)){
        return res.status(403).send({status: "forbidden", error: "a token is required to authenticate"});
    }
    if(!Boolean(the_key)){
        return res.status(503).send({status: "no_key", error: "oops got not key, lol"});
    }
    try{
        const unverified= jwt.decode(token)
        if (!(Boolean(unverified) && Boolean(unverified.auth))){
            return res.status(401).send({status: "malformed_token", error: "auth info missing or ill formed in token payload"})
        }

        const decoded= jwt.verify(token, the_key, VERIFY_OPTIONS)
        req.body.decoded_token= decoded
    }
    catch(err){
        hereLog(`Error: ${err}`)
        if (Boolean(err) && err.name==='TokenExpiredError'){
            return res.status(401).send({status: "expired", error: "token expiration"});
        }
        else return res.status(401).send({status: "auth_error", error: "authentification error"});
    }

    return next();
}

module.exports= {prepareKeyFromKeyFile, verifyTokenFromReq}