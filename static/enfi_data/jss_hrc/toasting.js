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
	var xmlDoc, txt1, x1, i1;
	var xhttp = new XMLHttpRequest();
	xhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200){
			xmlDoc = this.responseXML;
			txt1="";
			x1=xmlDoc.getElementsByTagName("msg1");
			for (i1 = 0; i1 < x1.length; i1++) {
			txt1 = txt1 + x1[i1].childNodes[0].nodeValue + "<br>";}
			document.getElementById("mesaj1").innerHTML = txt1;
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