(function() {
		var url ="http://api.openweathermap.org/data/2.5/weather?q=Sydney.Australia";
		var weatherAPIkey = "6bb02aab2358f22bba8bd3d563e8dde1";
		var httpRequest;
		makeRequest();
		//send request
		function makeRequest() {
			httpRequest = new XMLHttpRequest();
			httpRequest.onreadystatechange = responseMethod;
			httpRequest.open('GET',url + '&appid=' + weatherAPIkey);
			httpRequest.send();
		}
	 	//handle response
	 	function responseMethod () {
	 		if (httpRequest.readyState == 4) {
	 			console.log(httpRequest.responseText);
	 			if (httpRequest.status == 200) {
	 				updateUISuccess(httpRequest.responseText);
	 			} else {

	 			}
	 		}
	 	}

	 	function updateUISuccess(responseText) {
	 		var response = JSON.parse(responseText);
	 		var conditions = response.weather[0].main;
	 		var degC = Math.floor(response.main.temp - 273.15);
	 		console.log(degC);
	 		$("#input-therm").text(degC + String.fromCharCode(176)+"C");
	 	}

	})();

updateTime();
function updateTime() {
	time = document.getElementById('input-lock');
	today = new Date
	$('#input-lock').text(today.getHours() + ":" + today.getMinutes());
};
window.setInterval(updateTime, 60000);