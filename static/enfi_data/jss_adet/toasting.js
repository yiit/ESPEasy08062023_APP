function toasting() {
	var x = document.getElementById('toastmessage');
	x.innerHTML = 'Submitted';
	x.className = 'show';
	setTimeout(function(){
		x.innerHTML = '';
		x.className = x.className.replace('show', '');
	}, 200);
}
setInterval(function() { getData(); }, 100);
function getData() {
	var xmlDoc, txt1, txt2, txt3, txt4, txt5, x1, x2, x3, x4, x5, i1, i2, i3, i4, i5;
	var xhttp = new XMLHttpRequest();
	xhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200){
			xmlDoc = this.responseXML;
			txt1="";
			txt2="";
			txt3="";
			txt4="";
			txt5="";
			x1=xmlDoc.getElementsByTagName("net");
			for (i1 = 0; i1 < x1.length; i1++) {
			txt1 = txt1 + x1[i1].childNodes[0].nodeValue + "<br>";}
			document.getElementById("mesaj1").innerHTML = txt1;

			x2=xmlDoc.getElementsByTagName("dara");
			for (i2 = 0; i2 < x2.length; i2++) {
			txt2 = txt2 + x2[i2].childNodes[0].nodeValue + "<br>";}
			document.getElementById("mesaj2").innerHTML = txt2;

			x3=xmlDoc.getElementsByTagName("brut");
			for (i3 = 0; i3 < x3.length; i3++) {
			txt3 = txt3 + x3[i3].childNodes[0].nodeValue + "<br>";}
			document.getElementById("mesaj3").innerHTML = txt3;

			x4=xmlDoc.getElementsByTagName("adet");
			for (i4 = 0; i4 < x4.length; i4++) {
			txt4 = txt4 + x4[i4].childNodes[0].nodeValue + "<br>";}
			document.getElementById("mesaj4").innerHTML = txt4;

			x5=xmlDoc.getElementsByTagName("adetgramaj");
			for (i5 = 0; i5 < x5.length; i5++) {
			txt5 = txt5 + x5[i5].childNodes[0].nodeValue + "<br>";}
			document.getElementById("mesaj5").innerHTML = txt5;
		}
	};
	xhttp.open('GET', 'xml', true);
	xhttp.send();
}