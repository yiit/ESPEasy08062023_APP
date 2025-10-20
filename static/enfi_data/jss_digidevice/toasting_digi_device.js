function toasting() {var x = document.getElementById('toastmessage');x.innerHTML = 'Submitted'; x.className = 'show'; setTimeout(function(){x.innerHTML = ''; x.className = x.className.replace('show', ''); }, 2000);} </script><script>
setInterval(function() {
getData();
}, 1000);
function getData() {
var xmlDoc, txt1, txt2, x1, x2, i1, i2;
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {
if (this.readyState == 4 && this.status == 200){
xmlDoc = this.responseXML;
txt1="";
txt2="";


x1=xmlDoc.getElementsByTagName("pluadi");
for (i1 = 0; i1 < x1.length; i1++) {
txt1 = txt1 + x1[i1].childNodes[0].nodeValue + "<br>";}
document.getElementById("mesaj1").innerHTML = txt1;

x2=xmlDoc.getElementsByTagName("net");
for (i2 = 0; i2 < x2.length; i2++) {
txt2 = txt2 + x2[i2].childNodes[0].nodeValue + "<br>";}
document.getElementById("mesaj2").innerHTML = txt2;

}
};
xhttp.open('GET', 'xml', true);
xhttp.send();
}