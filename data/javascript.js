function btnClick(btn) {

    elementName = btn.id; elementValue = btn.value;
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) { console.log(this.responseText) }
    }
    xhttp.open('GET', 'btnClick?button=' + elementName + '&value=' + elementValue, true);
    xhttp.send();
};


function btnClickText(elementName, elementValue) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            console.log("btnClickText-->  "+this.responseText);
            //evaluate(this.responseText);
        }
    }
    //console.log (window.args); // NO EXISTE !!!!! PORQUE ???
    xhttp.open('GET', 'btnClick?button=' + elementName + '&value=' + elementValue + '', true); xhttp.send();
}


// var connection = new WebSocket('ws://' + location.host + ':81');
// connection.onopen = function () { console.log('Connection open!'); }
// connection.onclose = function () { console.log('Connection closed'); }
// connection.onerror = function (error) { console.log('Error detected: ' + error); }
// connection.onmessage = function (e) {
//     var server_message = e.data;
//      //console.log(e.data); 		//													HERE TURN ON AND OFF LOOGING OF EVAL
//     //eval(server_message);
// }

var intervalID = setInterval(function(){
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            //console.log("Periodic Function---  Response:  "+this.responseText);
            //eval(this.responseText);
            var myObj = JSON.parse(this.responseText);
             evaluate(myObj);
        }
    }
    //console.log (window.args); // NO EXISTE !!!!! PORQUE ???
    xhttp.open('GET', "/getJavaScript", true); xhttp.send();
}, 1000);
function evaluate(myObj) {
    for (const key in myObj){
        if(myObj.hasOwnProperty(key)){
            //if (key=="mtrLogger")
            //console.log(`${key} : ${myObj[key]}`);
            //console.log( myObj[key].slice(0,6));
            if (myObj[key]=="chkBxON") 
                {document.getElementById(key).checked=true;}
            else if (myObj[key]=="chkBxOFF") 
                {document.getElementById(key).checked=false;}
            else if ( myObj[key].slice(0,6)=="value=")
                document.getElementById(key).value=myObj[key].slice(6);
            else if ( myObj[key].slice(0,4)=="max=") 
                document.getElementById(key).max=myObj[key].slice(4);
            else if ( myObj[key].slice(0,4)=="min=") 
                document.getElementById(key).min=myObj[key].slice(4);
            else if ( myObj[key].slice(0,4)=="log=") 
                console.log(myObj[key].slice(4));
            else if ( myObj[key].slice(0,8)=="btnText=") {
                document.getElementById(key).value=myObj[key].slice(8);
                document.getElementById(key).textContent=myObj[key].slice(8);
            }
            else if ( myObj[key].slice(0,9)=="redirect=")
                {
                    console.log( myObj[key].slice(9));
                    //window.location.assign(myObj[key].slice(9));
                }
            else
            {
                console.log("Falled trough in Javascript.js::evaluate() with Key "+key+" : "+myObj[key]);
                document.getElementById(key).innerHTML = myObj[key];
            }
        }
    }
}


var timeFormat = 'MM/DD/YYYY HH:mm';
var color = Chart.helpers.color;
//var ctx = document.getElementById('canvasGraphic').getContext('2d');


//var canvasGraphic;
//var datos = parseCSV("time,relay,temp,analog\n11111111,0,20,7\n11111122,0,21,7.5\n11111133,0,22.5,8\n");
//var myChart;

// window.onload = function(){
//     // ....
//     var now = new Date();
//     btnClickText("Time",now.getTime()/1000);
//     this.console.log("window.onLoad()");
//     //var ctx = document.getElementById('canvasChart').getContext('2d');

//     myChart = new Chart( ctx , config );
//     this.console.log("chart:");
//     this.console.log(myChart);
//     this.console.log(ctx);

//     //data=loadCSV();
// };
let config = new Array;


function createConfig(name) {
    config.push({
        chartName: name,

        maintainAspectRatio: false, // esto es para que no shaga resize en los celulares
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                //label: 'Medicion',
                backgroundColor: color("red").alpha(0.5).rgbString(),
                borderColor: color("green").alpha(1).rgbString(),
                fill: false,
                //pointRadius: 5,
                pointStyle: 'circle',
                data: [
                ],
            }]
        },
        options: {
            showLines: true,
            legend: {
                display: false
            },
            title: {
                text: 'Capturas realizadas',
                display: false,
                fontSize: 16
            },
            layout: {
                padding: 5
            },
            scales: {
                xAxes: [{
                    type: 'time',
                    distribution: 'linear',
                    display: 'true',
                    scaleLabel: {
                        display: true,
                        labelString: 'Time'
                    },
                    offset: true,
                    scaleBeginAtZero: false

                }],
                yAxes: [{
                    scaleLabel: {
                        display: true,
                        labelString: 'Value'
                    }
                }]
            },
        }
    });
    return config;
}
function checkConfig(name) {
    ret = -1;
    config.forEach(function (conf, index) {
        //console.log(config.chartName);
        if (conf.chartName == name) {
            console.log("Looking for:"+name+"  Found "+conf.chartName+" at "+index);
            ret = index;
        }
    });
    return ret;
}

function loadCSV(file, id , limit = 100000) {
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function () {
        if (this.readyState == 4 && (this.status == 200 || this.status == 404)) {
         parseCSV(this.responseText, id, limit);
            if (id=='chartdCapt') window.chartdCapt.update();
            else window.chartLogger.update();
            //return dataArray;

        }
    };
    xmlhttp.open("GET", file, true);
    xmlhttp.send();
}


function parseCSV(string, id, linesLimit) {
    console.log("parceCSV()");
    for ( i=0; i < config.length; i++) {
        console.log("Inside Loop");

        var element = config[i];
        console.log("id:" + id + " chartName: " + element.chartName);
        if (element.chartName == id) {

            console.log("ParseCSV function has found match !!!");
            //console.log(string);
            var array = [];
            var items = [];
            var colors = ['cyan', 'red', 'green', 'blue', 'magenta', 'lightgray', 'purple', 'orange'];
            var lines = string.split("\n");
            var titles = lines[0].split(",");
            console.log(titles);
            //element.data = [];
            //element.data.labels = [];
            for (var i = 1; i < titles.length; i++) {
                element.data.datasets[i - 1] = [];
                element.data.datasets[i - 1].data = [];
                element.data.datasets[i - 1].backgroundColor = color(colors[i]).alpha(0.5).rgbString();
                element.data.datasets[i - 1].borderColor = color(colors[i]).alpha(0.5).rgbString();
                element.data.datasets[i - 1].datafill = false;
                element.data.datasets[i - 1].label = titles[i];
                 }
            //element.data.labels = titles;
            var loopStart = 1;
            if (lines.length > linesLimit) loopStart = lines.length - linesLimit;

            for (var i = loopStart; i < lines.length; i++) {
                var data = lines[i].split(",");
                var offset = new Date().getTimezoneOffset();
                element.data.labels.push(new Date(data[0] * 1000 + 60 * offset)); // add 5 hours for time zone, this should be automatic
                for (var j = 1; j < data.length; j++) {
                    element.data.datasets[j - 1].data.push(data[j]);
                }
                //console.log(element.data.datasets[0].data);
                //array.push(items[i]);

            }
            // element.data.datasets[0].data =(data1);
            // element.data.datasets[1].data =(data2);
            // element.data.datasets[2].data = (data3);
            //	element.data.labels=data0;
            element.options.scales.xAxes[0].time.min = element.data.labels[0];
            console.log(element);
            //return array;

        }
    }
}




function handlePrompt(id, text) {        // handles Dialogs and returns button click id.
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) { console.log(this.responseText); }
    }
    xhttp.open('GET', 'rename?file=/capturas' + id + '&name=/capturas/' + prompt(text) + ".csv", true);
    xhttp.send();
}
function handlePromptButton(id, text) {        // handles Dialogs and returns button click id.
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) { console.log(this.responseText); }
    }
     response = (prompt(text)); console.log("Button got "+response);
    xhttp.open('GET', 'btnClick?button=' + id + '&value=' + response, true);
    xhttp.send();
}

function handleUpload(fileName) {
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function () {
        if (this.readyState == 4 && (this.status == 200 || this.status == 404)) {
            //var dataArray = parseCSV(this.responseText);
            // var jsonString="";
            // for (var i = 0; i <  dataArray.length - 1; i++) {
            //     //console.log(dataArray[i][4]);
            //     //jsonString +=  "{\n\tcontainer_id:"+dataArray[i][2]+",\n\tdata:\n\t\t{\n\t\t\t2:"+dataArray[i][3]+",\n\t\t\t4:"+dataArray[i][4]+",\n\t\t\t5:"+dataArray[i][5]+",\n\t\t\t7:"+dataArray[i][6]+",\n\t\t},\n}\n\n"
            // }
            var http = new XMLHttpRequest();
            // //var url = "http://jsonplaceholder.typicode.com/users";
            var url = "http://httpbin.org/post";
            // //http://httpbin.org/post
            // var params = jsonString;
            http.open('POST', url, true);

            //Send the proper header information along with the request
            http.setRequestHeader('Content-type', 'application/csv;charset=UTF-8');
            http.setRequestHeader("auth_api_key", "1234567890");
            //http.setRequestHeader("auth_farm_hash",dataArray[0][1]);


            http.onreadystatechange = function () {//Call a function when the state changes.
                if (http.readyState == 4 && http.status == 200) {
                    alert(http.responseText);
                }

            }
            //console.log(JSON.stringify(dataArray));
            //console.clear();
            console.log("Sent to Server: " + fileName);
            http.send((fileName));
        }
    };
    xmlhttp.open("GET", fileName, true);
    xmlhttp.send();
}

//function checkForMatch(value,index,array){if (value.chartName==)}
function btnClickedPlus(id = config[0].chartName) {
    config.forEach(element => {
        console.log(element.chartName); if (element.chartName == id) {
            var span = calculateDataSpan(element.index); console.log(span);
            element.options.scales.xAxes[0].time.min = moment(element.options.scales.xAxes[0].time.min).add((span / 10));
            element.options.scales.xAxes[0].time.max = moment(element.options.scales.xAxes[0].time.max).subtract((span / 10));
        }
        window.myChart.update();
    });
}
function btnClickedMinus() {
    var span = calculateDataSpan(); console.log(span);
    config.options.scales.xAxes[0].time.min = moment(config.options.scales.xAxes[0].time.min).subtract(span / 10);
    config.options.scales.xAxes[0].time.max = moment(config.options.scales.xAxes[0].time.max).add(span / 10);
    window.myChart.update();
}
function btnClickedForward(id) {
    config.forEach(element => {
        if (element.chartName == id) {
            var span = calculateDataSpan(); console.log(span);
            element.options.scales.xAxes[0].time.min = moment(element.options.scales.xAxes[0].time.min).add(span / 10);
            element.options.scales.xAxes[0].time.max = moment(element.options.scales.xAxes[0].time.max).add(span / 10);
        }
        window.myChart.update();
    });
}
function btnClickedBackward(id) {
    config.forEach(element => {
        if (element.chartName == id) {
            var span = calculateDataSpan(); console.log(span);
            element.options.scales.xAxes[0].time.min = moment(element.options.scales.xAxes[0].time.min).subtract(span / 10);
            element.options.scales.xAxes[0].time.max = moment(element.options.scales.xAxes[0].time.max).subtract(span / 10);
        }
        window.myChart.update();
    });

}
function btnClickedCropEnd() {
    config.data.labels.pop();
    config.data.datasets.forEach(function (dataset, index) {
        dataset.data.pop();
    });
    window.myChart.update();
}
function btnClickedCropBegining() {
    config.data.labels.splice(0, 1);
    config.data.datasets.forEach(function (dataset, index) {
        dataset.data.splice(0, 1);
    });
    config.options.scales.xAxes[0].time.min = config.data.labels[0];

    window.myChart.update();
}
function calculateDataSpan(ind) {
    return moment(config[ind].options.scales.xAxes[0].time.max) - moment(config[ind].options.scales.xAxes[0].time.min);
}

