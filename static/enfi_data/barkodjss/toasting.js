function toasting() {var x = document.getElementById('toastmessage');x.innerHTML = 'Submitted'; x.className = 'show'; setTimeout(function(){x.innerHTML = ''; x.className = x.className.replace('show', ''); }, 200);} </script><script>
setInterval(function() {
getData();
}, 100);
function getData() {
var xmlDoc, txt1, x1, i1;
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {
if (this.readyState == 4 && this.status == 200){
xmlDoc = this.responseXML;
txt1="";


x1=xmlDoc.getElementsByTagName("barkod");
for (i1 = 0; i1 < x1.length; i1++) {
txt1 = txt1 + x1[i1].childNodes[0].nodeValue + "<br>";}
document.getElementById("mesaj1").innerHTML = txt1;



}
};
xhttp.open('GET', 'xml', true);
xhttp.send();
}