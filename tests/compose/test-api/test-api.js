const express= require('express');
const bodyParser= require('body-parser');



let hereLog= (...args) => {console.log("[test-api]", ...args);};


const {prepareKeyFromKeyFile, verifyTokenFromReq}= require("./token_stuff")

const fs = require('fs');

const app= express();

prepareKeyFromKeyFile();

const PORT=8080

// support parsing of application/json type post data
app.use(bodyParser.json());

//support parsing of application/x-www-form-urlencoded post data
app.use(bodyParser.urlencoded({ extended: true }));

app.listen(PORT, ()=>console.log(`listening on ${PORT}`));

app.get("/stop", (req, res) => {
    res.send({response: "bye!"});

    hereLog("stopping…")

    process.exit();
})

app.get("/simple_get1", (req, res) => {
    console.log("EntryPoint '/simple_get1' reached -> Sending...")
    res.send({
        result: "okidoki"
    })
})

app.get("/get_param_read", (req, res) => {
    console.log("EntryPoint '/get_param_read' reached -> Sending...")
    var {param1, param2} = req.query;

    res.send({
        response1: `got ${param1}`,
        resp2: `is ${param2}`
    });
})

app.put("/put/bodyTest", (req, res) => {
    console.log("EntryPoint '/put/bodyTest' reached -> Sending...")

    let body_param= req.body.param

    res.status(200).send({
        anwser: `putted ${body_param}`
    });
})

app.post("/post/headerTest", (req, res) => {
    console.log("EntryPoint '/post/headerTest' reached -> Sending...")

    let header_post= req.headers["x-test-header"]
    let header_post_b= req.headers["x-another-header"]

    res.status(200).send({
        result: `confirmed ${header_post}-${header_post_b}`
    });
})

app.put("/put/tokenBodyTest", verifyTokenFromReq, (req, res) => {
    console.log("EntryPoint '/put/tokenBodyTest' reached -> Sending...")

    res.status(200).send({
        result: "ok with token"
    });
})

try {
    fs.writeFileSync('/app/locker/wait', "go");
} catch (err) {
    console.error(err);
}
