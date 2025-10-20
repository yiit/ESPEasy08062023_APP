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
	var xmlDoc, txt1, txt2, txt3, x1, x2, x3, i1, i2, i3;
	var xhttp = new XMLHttpRequest();
	xhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200){
			xmlDoc = this.responseXML;
			txt1="";
			txt2="";
			txt3="";
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
		}
	};
	xhttp.open('GET', 'xml', true);
	xhttp.send();
}
function myFunction() {
 var input, filter, table, tr, td, i, txtValue;
  input = document.getElementById("myInput");
  filter = input.value.toUpperCase();
  table = document.getElementById("myTable");
  tr = table.getElementsByTagName("tr");
  for (i = 0; i < tr.length; i++) {
   td = tr[i].getElementsByTagName("td")[1];
   if (td) {
    txtValue = td.textContent || td.innerText;
    if (txtValue.toUpperCase().indexOf(filter) > -1) {
     tr[i].style.display = "";
    } else {
      tr[i].style.display = "none";
   }
  }
 }
}