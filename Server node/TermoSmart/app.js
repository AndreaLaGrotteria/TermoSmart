var express = require('express');
var bodyParser = require('body-parser');
var path = require('path');
var mongojs = require('mongojs');
var assert = require('assert');

var app = express();
var PORT = process.env.PORT || 8000;

var url = process.env.MONGO_URL;
var db = mongojs(url, ['temperatures']);

var token = process.env.TOKEN;

app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'ejs');

app.use(bodyParser.urlencoded({extended:false}));
app.use(bodyParser.json());

app.use(express.static(path.join(__dirname, 'public')));

app.get('/', function(req, res){

    db.temperatures.find({}).sort({_id:-1}).limit(240, function(err, result){
        //console.log(result);
        res.render('index', {
            temperatures: result
        });
    });
});

app.post('/update', function(req, res){

    if(req.body.token == token){
        var obj = { tSet: req.body.tempSet, tRec: req.body.tempRec, dateAdded: new Date() };
        db.temperatures.save(obj, function(err, res){
            if (err) throw err;
        });

        db.temperatures.find({}, function(err, result){
            if (result.length > 1200){
                for(var i=0; i < (result.length - 1200); i++){
                    db.temperatures.remove({ _id: result[i]._id});
                }
            } 
        });
    
        db.temperatures.find({ webUpdate: "not sent" }).sort({_id: -1}).limit(1, function(err, result){

            if(result[0]){
                res.send(result[0].tSet);
                db.temperatures.update({ _id: result[0]._id}, { webUpdate: "sent"});

            } else{
                res.send("ok");
            }
            
            
            
        });

        
    } else{
        res.send("401");
    }


});

app.post('/update/client', function(req, res){
    console.log(req.body.set__value);

    var obj = { tSet: req.body.set__value, dateAdded: new Date(), webUpdate: "not sent" }
    
    res.redirect('/');

    db.temperatures.save(obj, function(err, res){
        if (err) throw err;
    });
})

app.listen(PORT, function(err,res){
    if(err){
        console.log('server error');
    } else{
        console.log('server started');
    }
});