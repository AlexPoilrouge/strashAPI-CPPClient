const express= require('express');
const bodyParser= require('body-parser');

const fs = require('fs');

const app= express();


const PORT=8080

// support parsing of application/json type post data
app.use(bodyParser.json());

//support parsing of application/x-www-form-urlencoded post data
app.use(bodyParser.urlencoded({ extended: true }));

app.listen(PORT, ()=>console.log(`listening on ${PORT}`));


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

try {
    fs.writeFileSync('/app/locker/wait', "go");
} catch (err) {
    console.error(err);
}

