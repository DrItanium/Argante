
 ------------------------------------------------------------------------
 WARNING: This is an obsolete and no longer maintained version of Argante
 README. Only english version of this document is maintained, while this
 version is incomplete, inaccurate and certainly not up-to-date. It is
 provided for historical reasons *ONLY* and under no circumstances should
 be used as an authoritative source of information.
 ------------------------------------------------------------------------

 ---------------------------------------------------------------------------
 UWAGA: Ponizej znajduje sie nieaktualna i nie wspierana dalej wersja
 dokumentacji systemu Argante. Tylko wersja anglojezyczna jest uaktualniana,
 podczas gdy ponizszy dokument jest niekompletny i z pewnoscia niezgodny
 z obecnym stanem prac. Jest dostarczony tylko ze wzgledow historycznych
 i pod zadnym pozorem nie moze byc uzywany jako autorytatywne zrodlo
 informacji o projekcie.
 ---------------------------------------------------------------------------


               "[We] use bad software and bad machines for the wrong things."
							-- R.W. Hamming

    ___   ___   ___   ___   ___  |_   ___
   '___| |   ` |   | '___| |   | |   |___|
   |___| |     |___| |___| |   | |__ |___.
               .___|       
                             wersja 0.008

(C) 2000 Michal Zalewski <lcamtuf@tpi.pl>
(C) 2000 Argante Development Team <argante@cgs.pl>

Argante Development Team:

	  Michal Zalewski <lcamtuf@tpi.pl>
	  Maurycy Prodeus <z33d@eth-security.net>
	  Bulba <bulba@intelcom.pl>
	  Artur Skura <arturs@people.pl>
          Marcin Dawcewicz <miv@interia.pl>

Our website and mailing list:

          Download: http://lcamtuf.na.export.pl/arg.tgz
          Homepage: http://agt.buka.org
          Listserv: mail -s 'subscribe' argante-request@cgs.pl </dev/null
          Archive: http://argante.buka.org


===========
SPIS TRE�CI
===========

 1. Wst�p i podzi�kowania
 2. Po co powsta� Argante? Czym si� r�ni od innych?
 3. SMTA - model wieloprocesowo�ci
 4. VCPU - wirtualna architektura
 5. Zestaw polece� RSIS
 6. Wyj�tki niskopoziomowe (LLX)
 7. HAC - system kontroli przywilej�w
 8. SVFS - architektura systemu plik�w
 9. IPC/rIPC - komunikacja mi�dzyprocesowa <w przygotowaniu>
10. Skrypty i zarz�dzanie konsolowe
11. Pos�ugiwanie si� RSIS-assemblerem
12. Pos�ugiwanie si� translatorem AHLL <w przygotowaniu>
13. Specyfikacja modu��w standardowych
14. Tworzenie modu��w
15. Format plik�w wykonywalnych 
16. Wbudowany debugger
17. FAQ

99. DO ZROBIENIA


-------------------------
 1. Wst�p i podzi�kowania
-------------------------

"Argante" jest - stworzonym w du�ej mierze w przeci�gu kilku dni - wirtualnym
systemem operacyjnym. W obecnej fazie wdra�ania, ja zajmuje si� wi�kszo�ci�
rzeczy i nadzoruj� rozw�j kodu, aczkolwiek mam nadziej�, �e wiele innych
os�b przy��czy si� do realizacji tego projektu :)

Dlaczego ten system powsta� i czym r�ni si� od innych rozwi�za� dowiecie
si� w sekcji 2 tego tekstu, a w mi�dzyczasie chcia�bym podzi�kowa� wszystkim,
kt�rzy cho�by w najmniejszym stopniu przyczynili si� do obecnego kszta�tu
systemu (celowo pomijam tu ludzi, kt�rzy s� wsp�autorami kodu i zostali
wymienieni wcze�niej):

  Maja :)                       - za to, �e jest i ju�...
  S�awomir Krawczyk             - za jak zwykle zgry�liwe uwagi ;)
  Agnieszka S�ota               - za zainteresowanie pomys�em i trosk�
  Filip Niedorezo               - za Pierwszy Niezale�ny Program ;>
  Marek Bia�og�owy              - za "przera�asz mnie"
  Wojciech Purczy�ski           - za mi�� polemik�
  Jarek Sygitowicz              - za plany zaw�adni�cia �wiatem
  negativ			- dobre pomys�y :)
  eloy                          - mailing lista :)
  maxiu				- pomys�y, pomys�y na optymalizacje
  Zbyszek Sobiecki		- za network i zainteresowanie :)
  Piotr �urawski		- :)
  Lukasz Jachowicz		- za "drukowanie dokumentacji" :>

Ta lista jest do�� kr�tka - mam nadziej�, �e si� to zmieni :) Je�li czujesz
si� na niej pomini�ty, napisz mi. Napisz tak�e je�li uwa�asz, �e ten projekt
jest ciekawy lub je�li masz co do niego krytyczne uwagi, pomys�y etc. Ka�da
opinia jest bardzo cenna. Przede wszystkim jednak istotne jest to, by�
zapozna� si� z ca�o�ci� tego dokumentu, i najpierw spr�bowa� odnale��
odpowiedzi na pytania, kt�re Ci si� nasuwaj�. Dla programist�w przyzwyczajonych
do Unix�w, do klasycznego assemblera i do tradycyjnych konstrukcji, wiele
rzeczy mo�e wyda� si� na pierwszy rzut oka "bezsensowna" - ale zapewniam
Was, �e niemal ka�dy element Argante ma swoje uzasadnienie, i �e przy
odrobinie dobrej woli odnajdziecie je tutaj :)

Cenna literatura:

  Steven Muchnick, "Advanced Compiler Design and Implementation"
  Andrew S. Tanenbaum, "Distributed Operating Systems"
  Doreen L. Galli, "Distributed Operating Systems - concepts & practice"
  Andrew S. Tanenbaum, "Modern Operating Systems"
  Eric S. Raymond, "The Cathedral & the Bazaar"
  Illiad, "Evil Geniuses in a Nutshell"

  http://www.vitanuova.com/inferno/papers/bltj.html


  (wi�cej nie mam pod r�k� ;)


----------------------------------------------------
 2. Po co powsta� Argante? Czym si� r�ni od innych?
----------------------------------------------------

Argante jest w pe�ni wirtualnym �rodowiskiem uruchamiania aplikacji w
obr�bie system�w Unixowych. Wielu ludziom mo�e nasuwa� to skojarzenie np.
z Jav� i jej sandbox'em, aczkolwiek za�o�enia techniczne, kt�re sta�y si�
podstawami Argante by�y diametralnie r�ne.

Przede wszystkim, Argante jest pe�nym systemem operacyjnym. Dysponuje
w�asn� implementacj� proces�w, komunikacji mi�dzyprocesowej, systemu
plik�w, modelu uprawnie�... Po co to wszystko? Postaram si� wyja�ni�:

Standardowa architektura system�w oraz sprz�tu (np. procesor�w) pozostawia 
bardzo wiele do �yczenia je�li chodzi o bezpiecze�stwo i stabilno��
oprogramowania. W skr�cie - brak niskopoziomowego wsparcia szeroko poj�tej
kontroli uprawnie�, obs�ugi b��d�w (prymitywne techniki udost�pniane przez
procesory np. serii 80386 napewno same w sobie nie wystarczaj�), niezbyt 
fortunne za�o�enia leg�y u podstaw architektury u�ytkowania segment�w stosu 
czy danych. 

Naprawianie tych b��d�w na wy�szym poziomie jest na og� niepewne i
ryzykowne. Autorzy Javy doprowadzili do stworzenia �a�o�nie wolnego i w gruncie
rzeczy nie zawsze bezpiecznego rozwi�zania o bardzo ograniczonym zakresie
stosowania, poza tym nie byli w stanie wymusi� na autorach oprogramowania
korzystania z bezpiecznych, sprawdzonych modeli architektury - jak na przyk�ad
OSI, architektura ograniczonego zaufania i interakcji, kt�ra zak�ada, i� tylko
najbli�sze sobie warstwy przetwarzania danych wsp�pracuj� ze sob�, a kod
podzielony jest na segmenty funkcjonalne. Pisane w C programy wykorzystuj�ce
model "listener -> fork() -> client handling" s� wci�� prostsze do wdro�enia
i bardziej niezawodne.

Tak wi�c powsta�a w wyobra�ni lista tych i wielu innych uwag pod adresem 
popularnego modelu architektury sprz�towej i programowej. Jej przes�anie
najlepiej wyra�a motto znajduj�ce si� na pocz�tku tego dokumentu:

"[We] use bad software and bad machines for the wrong things."
					    -- R.W. Hamming

Opr�cz tego, poza skargami, nasun�o mi si� wiele pomys��w, kt�re moim
zdaniem powinny - i, niewielkim kosztem, mog�yby - zosta� uwzgl�dnione przy
projektowaniu rozwi�za� na ka�dym z tych dw�ch poziom�w - hardware i
software.

Stan��em w pewnej chwili przed trudn� decyzj�. Mog�em modyfikowa� istniej�ce 
rozwi�zania, pr�buj�c tworzy� prowizoryczne "�aty" gdzie popadnie, nara�aj�c 
si� na to, i� wi�kszo�� pomys��w nie da si� w pe�ni zrealizowa� - a tak�e na 
to, i� realizacja projektu stanie si� seri� kompromis�w, po�r�d kt�rych 
umknie gdzie� sens jego realizacji :) Mog�em te� zrobi� co� innego - usi��� i 
napisa� wszystko od pocz�tku. Zapominaj�c o zgodno�ci, o konwencjach, chc�c 
stworzy� rozwi�zanie, kt�re b�dzie umia�o obroni� si� samo, albo na kt�re nikt 
nie zwr�ci uwagi :) Tak narodzi� si� pomys� Argante, z czterema g��wnymi
postulatami:

- bezpiecze�stwo i stabilno��
- funkcjonalno��
- wydajno��
- prostota

Argante mia� by� systemem bez kompromis�w. Dlatego w sytuacji, gdy w
tradycyjnym rozwi�zaniu stan�liby�my w obliczu wyboru pomi�dzy bezpiecze�stwem
a funkcjonalno�ci�, my zamiast wybiera� jeden z wariant�w, dochodzili�my do
wniosku, i� rozwi�zanie jest z�e, i tworzyli�my jego szkic od podstaw lub
zmieniali�my model tak, by pogodzi� stawiane wymagania i nasze oczekiwania.

Dlaczego Argante to system osadzony? Jest kilka powod�w. Pierwszy z nich
to to, i� rozwi�zanie osadzone nie wymusza zmiany systemu, pozwala na �atwe
przeprowadzanie pr�b i wst�pnych wdro�e�, w integracji z istniej�cymi
rozwi�zaniami na macierzystej platformie Unixowej. Argante dzi�ki temu
wprowadza dodatkow� warstw� ochrony i abstrakcji, dzia�aj�c tak, jak zupe�nie
niezale�na architektura sprz�towa, nie poci�gaj�c za sob� konieczno�ci
powa�nych zmian. J�zyk C zapewnia z kolei wydajno�� i przeno�no�� kodu.
Dzi�ki temu te�, implementacja, kt�ra mo�e odwo�ywa� si� do istniej�cych
sterownik�w systemowych, urz�dze�, funkcji systemowych, sta�a si� du�o
prostszym zadaniem i pozwoli�a na skupienie si� na aspektach merytorycznych,
a nie detalach implementacyjnych (typu bootloader, sterowniki, etc).

Oczywi�cie m�wi�c o stabilno�ci i bezpiecze�stwie systemu osadzonego mam
na my�li to, i� implementuje on niezale�ne od platformy macierzystej
systemy kontroli dost�pu, w�asny model wieloprocesowo�ci - i wszystkie
te rozwi�zania s� bezpieczne i niezale�ne od systemu rzeczywistego. Dlatego
Argante na niemal dowolnym Unixie (a mo�e i Windows?;) b�dzie rozwi�zaniem
bezpiecznym pod warunkiem, i� zostanie zapewnione elementarne bezpiecze�stwo
platformy macierzystej - w najprostszym wariancie, usuni�te zostan� wszystkie
us�ugi sieciowe (innym tematem s� rozwi�zania hybrydowe, kt�re zostan�
om�wione przy okazji rIPC oraz network).

Aby zrealizowa� wy�ej wymienione cztery za�o�enia, stworzy�em og�lne wytyczne
dotycz�ce systemu. Brzmia�y one nast�puj�co:

- j�dro systemu b�dzie mikrokernelem, zapewniaj�cym podstawow� funkcjonalno��;
  wszystkie operacje wej�cia/wyj�cia b�d� odbywa�y si� z wykorzystaniem
  �adowalnych modu��w, kt�re mog� by� w prosty spos�b wdra�ane przez
  u�ytkownika, a tak�e dodawane/usuwane w trakcie pracy systemu; modu�y mog�
  tak�e zawiera� np. funkcje biblioteczne zapewniaj�ce zaawansowane operacje
  na �ancuchach tekstowych czy podobne procedury,

- system b�dzie zapewnia� _dowoln�_ funkcjonalno��, pozwalaj�c� stworzy�
  w nim oprogramowanie pocz�wszy od serwera baz danych a ko�cz�c na aplikacji
  graficznej, bez konieczno�ci wprowadzania zmian w kodzie systemu, a
  r�wnocze�nie b�dzie zapewnia� najwy�sze bezpiecze�stwo,

- system b�dzie dysponowa� w�asnym, niskopoziomowym, niezale�nym od
  platformy sprzetowej j�zykiem maszyny wirtualnej; j�zyk b�dzie wystarczaj�co
  prosty i wydajny, by zapewni� szybk� i efektywn� prac�, a r�wnocze�nie
  b�dzie zapewnia� pe�ne oddzielenie od rzeczywistego systemu i nie b�dzie
  pozwala� na wykonywanie natywnego kodu maszyny,

- zarz�dzanie systemem b�dzie w pe�ni roz��czne wzgl�dem proces�w uruchamianych
  w obr�bie wirtualnego systemu; obowi�zywa� b�dzie tak�e pe�na rozdzielno��
  user-space i kernel-space bez mo�liwo�ci ingerencji w kernel-space z poziomu
  user-space.

- ka�dy proces uruchomiony w systemie b�dzie dysponowa� w�asn�, prywatn�
  przestrzeni� adresow�, wydzielonym segmentem stosu, kt�ry nie b�dzie m�g�
  by� bezpo�rednio adresowany (u�ywany tylko przez funkcje skoku/powrotu);
  podobnie b�dzie z segmentem kodu, kt�ry nie b�dzie bezpo�rednio adresowalny.
  Tylko segment kodu b�dzie wykonywalny.

- proces b�dzie m�g� alokowa� bloki pami�ci, roz��cznie mapowane do jego
  prywatnej przestrzeni adresowej (z mo�liwo�ci� ochrony przed zapisem);
  system b�dzie zapewnia� kontrol� pr�b naruszenia granic przydzielonego
  bloku (bufora),

- system b�dzie wspiera� niskopoziomow� obs�ug� wyj�tk�w i pozwala� programowi
  je obs�ugiwa� (LLX - low-level exceptions), 

- system b�dzie posiada� w�asn�, bezpieczn� i oszcz�dzaj�c� zasoby systemowe
  implementacj� multitaskingu oraz w�asny, statyczny model proces�w (SMTA)
  z przypisanymi na sta�e grupami przywilej�w; wspierane b�d� tak�e 
  zastosowania wielou�ytkownikowe przez mo�liwo�� okre�lania identyfikatora
  podgrupy w obr�bie danej domeny uprawnie�,

- nowa filozofia przyznawania / zrzucania uprawnie�, nie nios�ca ze sob�
  ryzyka, kt�rym obarczona jest Unixowa implementacja,

- system od pocz�tku b�dzie wspiera� bezpieczne rozwi�zania (typu
  unbounded strings w miejsce null-terminated, etc),

- system b�dzie zapewnia� implementacj� hierarchicznej, scentralizowanej
  i uniwersalnej hierarchicznej kontroli dost�pu (HAC) pozwalaj�cej na
  okre�lanie przywilej�w z dowolnym poziomem szczeg�owo�ci; dodatkowo,
  system wymusi architektur� "zwrotnicy", wymuszaj�c na programi�cie
  okre�lenie, kt�re uprawnienia potrzebne s� do wykonania danej operacji,
  i nie pozwalaj�cego na posiadanie �adnych innych,

- system b�dzie silnie wspiera� architektur� OSI, w tym tak�e architektur�
  rozproszon�, przez udost�pnienie zaawansowanego mechanizmu komunikacji 
  mi�dzyprocesowej IPC (w�asne rozwi�zanie, odmienne od Unixowego) oraz
  rIPC (remote IPC, dystrybuowanie sesji mi�dzy to�samymi procesami,
  transparentna dla user-space komunikacja mi�dzy zadaniami na r�nych
  komputerach); rIPC b�dzie te� wspiera� transparentn� architektur�
  clustrow�

- system b�dzie zapewnia� implementacj� w�asnego, wirtualnego systemu
  plik�w, dost�pnego z poziomu rzeczywistego systemu plik�w, a r�wnocze�nie
  pozwalaj�cego na ustalenie dowolnej struktury wewn�trznej i pe�n� kontrol�
  dost�pu zgodn� z HAC,

- zmiana dowolnego rodzaju funkcjonalno�ci b�dzie mo�liwa bez konieczno�ci
  przerywania pracy systemu

Argante sprzyja tworzeniu rozwi�za� hybrydowych - na przyk�ad aplikacje
rzeczywistego systemu koordynowane / chronione przez kod Argante. Dzi�ki
tym mo�liwo�ciom b�dzie mo�na tworzy� w spos�b transparentny redundantnych,
heterogenicznych clustr�w z mo�liwosci� morphingu, samodzielnej przydzielania
nowych obiektow w istniej�cej hierarchii i pe�na redundancja oraz load
balancingiem bez _�adnych_ nak�ad�w programistycznych. Nie ma r�nicy,
czy system b�dzie pracowa� na jednej maszynie, czy na stu, z reduntantnymi
rozwi�zaniami i load balancingiem - po prostu filozofia rIPC rozwi�zuje
problemy system�w rozproszonych w spos�b przejrzysty dla aplikacji.

Wiem, �e brzmi to jak pobo�na lista �ycze�, ale pisz� te s�owa ju� po
wdro�eniu wi�kszo�ci kodu systemu i ze zdumieniem (nieskromnie) stwierdzam, �e 
uda�o mi si� osi�gn�� te zamierzenia. Co uzyska�em?

- bezpiecze�stwo i stabilno��: 

  - praktycznie brak mo�liwo�ci przej�cia kontroli nad aplikacj� w systemie
    (kontrola stosu, segmentu danych, bufor�w, filozofia przekazywania
    parametr�w do syscalli nie wymuszona konwencjami C - typu null-term);
    ze wzgl�du na niewielki zestaw polece� maszynowych RSIS, kontrola
    uprawnie� jest aspektem banalnym,

  - sprzyjanie programowaniu zgodnemu z bezpieczn� architektur� OSI -
    po prostu jest to intuicyjne w tym systemie,

  - wymuszanie kontroli poprawno�ci wykonywania kodu przez zg�aszanie
    wyj�tk�w,

  - nawet je�li by�oby to mo�liwe, brak mo�liwo�ci zdobycia uprawnie� 
    pozwalaj�cych na naruszenie bezpiecze�stwa reszty systemu wirtualnego
    (oddzielenie zarz�dzania od systemu wirtualnego, odci�cie od kernel-pace),

  - a nawet gdyby to by�o mo�liwe, niemo�no�� wp�yni�cia na prac�
    rzeczywistego systemu (oddzielna implementacja multitaskingu, nie
    wykorzystuj�ca implementacji systemu rzeczywistego),

  - pe�na kontrola dost�pu do dowolnych zasob�w (HAC), wspomniana ju�
    nowa filozofia uprawnie�, nowa filozofia wi�zania przywilej�w z
    procesem, i nowy model proces�w, etc...

  - destabilizacja systemu macierzystego jest w�a�ciwie niemo�liwa,

  - wsparcie redundancji i dystrybucji zapyta�,

- funkcjonalno�� i prostota:

  - uniwersalno�� systemu przez zapewnienie wsparcia wygodnych modu��w
    i scentralizowanej kontroli, oraz efektywnej architektury wirtualnego
    procesora o niewielkim, lecz wydajnym zestawie polece�,

  - mo�liwo�� tworzenia system�w rozproszonych bez konieczno�ci modyfikacji
    kodu; mo�liwo�� propagowania request�w bez konieczno�ci modyfikacji
    kodu (mowa o kodzie aplikacji),

  - wyj�tki upraszczaj� obs�ug� b��d�w,

  - wprowadzanie nawet powa�nych zmian do systemu mo�e odbywa� si� w locie
    przez podmian� modu��w,

  - load balancing, tworzenie clustr�w, rozdzielanie rozwi�zania mi�dzy maszyny
    mo�e odbywa� si� bez modyfikacji kodu �r�d�owego jego element�w,

- wydajno��:

  - dzi�ki u�yciu niskopoziomowego kodu wirtualnego, zamiast - jak np.
    w przypadku Javy - kodu wysokopoziomowego - spadek wydajno�ci nie jest
    tak ra��cy, nie ogranicza si� te� mo�liwo�ci kodu. P�tle typu
    "idle loop" (tzn powtarzany w k�ko skok) wykonywa� si� kilkakrotnie
    wolniej, ni� skompilowany w C na natywnej platformie sprz�towej, co jest
    wynikiem bardzo dobrym. W przypadku bardziej z�o�onych operacji (np. I/O),
    spadek wydajno�ci jest znacznie mniejszy i waha si� w zakresie 15-30%.

  - zaimplementowany multitasking jest du�o bardziej stabilny i mniej
    pami�cio�erny ni� w przypadku natywnego systemu; wynika to po cz�ci
    z tego, �e wirtualny procesor Argante potrzebuje mniej informacji
    by "utrzyma�" proces ni� Unix, a po cz�ci z niedoskona�o�ci wielu
    system�w.

Chcieli�my po��czy� QNXa, HURDa, oraz wszystkie nasze "lu�ne" pomys�y by
stworzy� naprawde bezpieczne i efektywne rozwi�zanie :) P�niej, Pawe�
Krawczyk zwr�ci� nam uwag� na system osadzony Inferno, kt�ry cho� r�ni
si� od Argante, ma te� z ni� wiele wsp�lnego. Informacje o nim znajdziecie
pod adresem http://www.vitanuova.com/inferno/papers/bltj.html. System
by� wdra�any przez Lucenta.

Wierzymy, �e unikneli�my wielu niepe�nych rozwi�za�, jak na przyk�ad
przenoszenie wysokopoziomowej funcjonalno�ci na ni�sze poziomy bez dobrego
powodu; decydowali�my si� na takie posuni�cie bardzo rzadko, tylko gdy
byli�my pewni i� zaoferuje to rzeczywist� korzy�� dla systemu, bez
narzucania statycznych, z�o�onych rozwi�za� gdzie nie s� one potrzebne.

Uff, to tyle. Pora sko�czy� z marketingiem i przej�� do szczeg��w
implementacyjnych ;)

Aha, do�� prosty, ale radosny tutorial uruchamiaj�cy dwa programy
mo�na obejrze� wpisuj�c "./build test".


----------------------------------
 3. SMTA - model wieloprocesowo�ci
----------------------------------

Koncepcja proces�w w Argante mo�e dla wielu os�b wyda� si� szokuj�ca, zw�aszcza
je�li przywykli do Unixowego schematu, kt�ry zak�ada: jeden klient, jeden
proces. W Argante procesy s� obiektami statycznymi - zostaj� powo�ane do
�ycia z poziomu konsoli zarz�dzaj�cej lub skryptu. Przynajmniej wed�ug
standardowej semantyki Argante (oczywi�cie nic nie stoi na przeszkodzie, by
j� zmieni�, dodaj�c nowy syscall), procesy nie maj� mo�liwo�ci si� mno�y�,
tworzy� potomstwa lub wykonywa� w swoje miejsce inne programy.

Zamiast tego, Argante wspiera model OSI, gdzie procesowi przypisany jest
nie obiekt (jak na przyk�ad ��cz�cy si� klient), ale pewna funkcja (np.
obs�uga bazy danych czy obs�uga po��cze�). Cho� wydaje si� to by� dodatkowym
utrudnieniem, jestem pewien, i� gdy dotrzecie do ko�ca tego dokumentu,
przekonacie si�, i� nie jest to co� z�ego. Proces jest wczytywany w
przestrze� wirtualnego procesora - VCPU - i tam egzystuje, dop�ki nie 
zako�czy swojej pracy, lub nie przytrafi si� krytyczny b��d (typu
nieobs�u�ony wyj�tek).

Wi�kszo�� parametr�w procesu - jak na przyk�ad priorytet, czy te� zestaw
"domen" (kt�re s� obiektem zbli�onym to Unixowych supplementary groups)
jest przypisany do obrazu binarnego danego pliku wykonywalnego w chwili
kompilacji. Oto przyk�adowa struktura demona ftp, kt�ry spe�nia wymogi OSI
i jest bardzo prosty do zaimplementowania w Argante (by� mo�e prostszy
ni� w C), a do tego zdecydowanie bardziej bezpieczny i... wydajniejszy:

  TCP/IP                                         baza danych
    |                                                 |           "reality"
  --|-------------------------------------------------|----------------------
  (net) (ipc)-------(ipc)-(ipc)-(ipc)   (ipc)-(ipc)  (fs)       kernel space
  --|-----|-----------|-----|-----|-------|-----|-----|----------------------
    |     |           |     |     |       |     |     |           user space
   <A>----+          <B>   <B>   <B>------+    <C>----+

A - proces obs�uguj�cy po��czenia z sieci - odbiera po��czenia,
    ��czy si� z jednym z proces�w B po IPC i przekazuje im polecenia

B - procesy obs�uguj�ce klienta (dowolna ilo��, automatyczna propagacja
    zlece�); obs�uguj� polecenia, po IPC komunikuj� si� z procesem
    autoryzuj�cym; dzi�ki �atwo�ci u�ycia IPC i wsparciu kontekst�w
    obs�uga wielu sesji w jednym procesie nie jest problemem.

C - proces realizuj�cy autoryzacj�; korzystaj�c z modu�u fs weryfikuje
    wpisy w lokalnej bazie danych dost�pnej w SVFS.

To, jak realizowana jest komunikacja mi�dzy procesami, jak dzia�aj�
uprawnienia i zmiana grup, powiemy sobie przy omawianiu systemu HAC. Na
razie wypada nadmieni�, i� procesy nie b�d� mia�y nigdy r�wnocze�nie
mo�liwo�ci np. wykonywania operacji z wykorzystaniem modu�u net i ipc
(wspomniana wcze�niej "zwrotnica"), ani �e proces A nie b�dzie m�g� po IPC
nawi�za� po��czenia bezpo�rednio z C.

Po pierwsze, bezpiecze�stwo. Po drugie, z�o�ono�� programistyczna na
poziomie nie przekraczaj�cym napewno tej samej aplikacji w C. Po trzecie,
niewsp�miernie wi�ksza wydajno�� w por�wnianiu z modelem wykorzystuj�cym
fork().

Jak dzia�a multitasking? Generalnie, jest do�� sprawiedliwy. Ka�demu procesowi
w danym cyklu obs�ugi proces�w przys�uguje tyle cykli maszynowych jego
wirtualnego procesora, ile wynosi warto�� "priorytetu" procesu. Tak wi�c
proces o priorytecie 10000 otrzyma 10000 cykli, natomiast proces o priorytecie
1 - jeden cykl. Zalecane jest oczywi�cie nadawanie procesom rozs�dnych
warto�ci priorytetu z zakresu 100-10000.

Procesy mog� znale�� si� w stanie STATE_SLEEPFOR, w kt�rym ich wykonywanie
jest wstrzymywane na okre�lon� ilo�� cykli obs�ugi proces�w; mo�liwy jest
te� stan STATE_SLEEPTILL, gdy proces oczekuje okre�lon� ilo�� mikrosekund,
lub STATE_IOWAIT, gdy proces oczekuje np. na otrzymanie prawa zapisu do
pliku, do kt�rego w danej chwili zapisuje inny proces, lub na odebranie
danych z socketa (oczywi�cie tylko je�li za�yczy sobie zosta� wprowadzony
w ten stan, gdy� mo�e te� wywo�a� funkcj� z opcj� NONBLOCK).


---------------------------------
 4. VCPU - wirtualna architektura
---------------------------------


Technicznie rzecz bior�c, VCPU jest wirtualn� maszyn�, udost�pniaj�c�
niewielki, lecz wygodniejszy w u�yciu od tradycyjnego kodu maszynowego
zestaw instrukcji, trzy bloki rejestrowe (odpowiednio do operacji na
32-bitowych warto�ciach ca�kowitych bez znaku, ze znakiem, oraz dla
liczb zmiennoprzecinkowych) po 16 rejestr�w, przestrze� stosu (wykorzystywan�
wy��cznie przy wywo�aniach funkcji; do przechowywania danych s�u�� inne
rozwi�zania), numer identyfikacyjny w komunikacji mi�dzyprocesowej IPC/RIPC,
przestrze� wykonywania programu, przestrze� przeznaczon� na alokacj� blok�w 
pami�ci, z implementacj� mechanizmu kontroli dost�pu i dynamicznej
realokacji, oraz kilka innych, mniej istotnych zmiennych. Alokowane bloki 
pami�ci, kt�re mog� by� u�ywane do przechowywania i obr�bki danych, s� 
roz��czne i nie jest mo�liwe przepisanie innego bloku przy przekroczeniu 
granicy poprzedniego. Nie jest tak�e mo�liwe modyfikowanie stosu oraz kodu 
programu, o czym wspomina�em wcze�niej.

Adresowanie pami�ci alokowalnej odbywa si� nie jak w standardowych systemach
w systemie 8-bitowym, ale z wykorzystaniem dwords, 32-bitowych skok�w. Oznacza
to wy�sz� wydajno�� w wi�kszo�ci zastosowa�, i przy okazji bezpieczniejszy
dost�p do danych.

Podobnie z adresowaniem przestrzeni kodu. Ka�da instrukcja kodowana jest
za pomoc� 12 bajt�w. Instrukcje, kt�re nie potrzebuj� argument�w, jak np. NOP,
maj� ustawiony tylko pierwszy bajt oznaczaj�cy opcode. W innym przypadku,
kolejne dwa bajty oznaczaj� rodzaj parametru, jeden bajt jest "paddingiem",
oraz dwa kolejne bajty zawieraj� parametry. W�a�ciwie mo�na zamkn�� si� w
10 bajtach, ale tracimy wtedy na wydajno�ci (w obecnym uk�adzie mamy trzy
razy 32-bitowy dword).

Takie rozwi�zanie pozwala na bezpieczniejsze poruszanie si� w obr�bie
segmentu kodu (tak�e przy skokach) oraz na zmniejszenie ilo�ci opcod�w, a
tak�e na drastyczny zysk je�li chodzi o ilo�� instrukcji koniecznych do
wykonania danej operacji, co kompensuje rozmiar pojedynczej instrukcji:

       1     2     3     4     5     6     7     8     9     10    11    12
 +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
  xxxxx xxxxx xxxxx RSRVD xxxxxxxxxxxxxxxxxxxxxxx xxxxxxxxxxxxxxxxxxxxxxx
  |     |     |           |                       |
  |     |     |           +-----------------------+- dwa parametry 32-bitowe
  |     |     +------------------------------------- typ drugiego parametru
  |     +------------------------------------------- typ pierwszego parametru
  +------------------------------------------------- opcode, np. MOV

Typy parametr�w to:

IMMEDIATE - 32-bitowa warto�� liczbowa
UREG      - numer rejestru typu unsigned
SREG      - numer rejestru typu signed
FREG      - numer rejestru typu float
IMMPTR    - wska�nik liczbowy do 32-bitowej warto�ci liczbowej
UPTR      - numer rejestru typu unsigned zawieraj�cy wska�nik do 32-bitowej
            warto�ci liczbowej


Uwaga: w przypadku instrukcji skoku, podanie parametru typu IMMEDIATE
czy UREG oznacza po prostu adres. W przypadku instrukcji typu MOV jest
inaczej - je�li chcemy odwo�a� si� do adresu miejsca w pami�ci, powinni�my
pos�u�y� si� typem IMMPTR lub UPTR. Jest to umowna konwencja, pozwalaj�ca
na efektywniejsze u�ycie komend.

Dost�pne s� nast�puj�ce rejestry:

  u0 .. u15             - rejestry typu unsigned, bez znaku (0..15)
  s0 .. s15             - rejestry typu signed, ze znakiem  (100..115)
  f0 .. f15             - rejestry zmiennoprzecinkowe (200..215)

System zapewnia konwersj� przy operacjach na rejestrach, aczkolwiek jest
to zabieg czasoch�onny i nie powinien by� u�ywany zbyt cz�sto. Nie jest
przeprowadzana konwersja warto�ci pobieranych z pami�ci (a wi�c zapisuj�c
pod adres 1234 warto�� rejestru f0 = 0.123, a potem odczytuj�c warto�� z
tego adresu do rejestru u0, otrzymamy prawdopodobnie nieporz�dany wynik;
rozwi�zaniem jest wczytanie warto�ci ponownie do rejestru f0 i u�ycie
mov u0,f0).

Proces uruchomiony na VCPU mo�e pos�ugiwa� si� wy��cznie natywnym zestawem
instrukcji, okre�lonymi jako RSIS, bez mo�liwo�ci wykonywania bezpo�rednio
kodu maszynowego rzeczywistego procesora.


-----------------------
 5. Zestaw polece� RSIS
-----------------------

Oto wst�pny opis polece� maszynowych systemu:


Mnemonik:  NOP
Parametry: -
Opcode:	   0
Opis:	   nie r�b nic
Efekt:     -
Wyj�tki:   -


Mnemonik:  JMP <addr>
Parametry: <addr> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   1
Opis:	   skok bezwarunkowy pod adres absolutny
Efekt:	   zmiana IP
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  IFEQ <x> <y>
Parametry: <x> = dowolny, <y> = dowolny
Opcode:	   2
Opis:	   wykonanie nastepnego polecenia je�li <x> = <y>
Efekt:	   warunkowo zmiana IP
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  IFNEQ <x> <y>
Parametry: <x> = dowolny, <y> = dowolny
Opcode:	   3
Opis:	   wykonanie nastepnego polecenia je�li <x> != <y>
Efekt:	   warunkowo zmiana IP
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  IFABO <x> <y>
Parametry: <x> = dowolny, <y> = dowolny
Opcode:	   4
Opis:	   wykonanie nastepnego polecenia je�li <x> > <y>
Efekt:	   warunkowo zmiana IP
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  IFBEL <x> <y>
Parametry: <x> = dowolny, <y> = dowolny
Opcode:	   5
Opis:	   wykonanie nastepnego polecenia je�li <x> < <y>
Efekt:	   warunkowo zmiana IP
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  CALL <addr>
Parametry: <addr> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   6
Opis:	   skok bezwarunkowy pod adres absolutny z od�o�eniem adresu
Efekt:	   zmiana IP, od�o�enie adresu na stos
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT, STACK_OVER


Mnemonik:  RET <cnt>
Parametry: <cnt> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   7
Opis:	   powr�t pod <cnt> adres zdj�ty ze stosu
Efekt:	   zmiana IP, pobranie ze stosu
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT, STACK_UNDER


Mnemonik:  HALT
Parametry: -
Opcode:	   8
Opis:	   zako�czenie pracy VCPU; tak�e w trybie respawn
Efekt:	   -
Wyj�tki:   -


Mnemonik:  SYSCALL <nr>
Parametry: <nr> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   9
Opis:      wywo�anie syscalla
Efekt:	   zale�ny od syscalla
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT, NOMODULE + 
           zale�ne od syscalla


Mnemonik:  ADD <x> <y> - opcode 10
           SUB <x> <y> - opcode 11
           MUL <x> <y> - opcode 12
           DIV <x> <y> - opcode 13
           MOV <x> <y> - opcode 19
Parametry: <x> = UREG, FREG, SREG, IMMPTR, UPTR
           <y> = IMMEDIATE, UREG, FREG, SREG, IMMPTR, UPTR
Opis:      operacje arytmetyczne (+, -, *, /, przypisanie)
Efekt:	   zmiana warto�ci pierwszego argumentu
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  MOD <x> <y> - opcode 14
           XOR <x> <y> - opcode 15
	   REV <x> <y> - opcode 16
           AND <x> <y> - opcode 17
           OR  <x> <y> - opcode 18
Parametry: <x> = UREG, SREG, IMMPTR, UPTR
	   <y> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opis:	   operacje binarne
Efekt:	   zmienia warto�� pierwszego argumentu
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  CWAIT <x>
Parametry: <x> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opcode:    20
Opis:      usypia proces na <x> takt�w SMTA
Efekt:     -
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  TWAIT <x>
Parametry: <x> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opcode:    21
Opis:      usypia proces na [conajmniej] <x> mikrosekund
Efekt:     -
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  ALLOC <size> <prot>
Parametry: <size> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           <size> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opcode:    22
Opis:      alokuje blok pami�ci o rozmiarze size i flagach uprawnie�
	   prot.
Efekt:     u0 - numer identyfikacyjny bloku, u1 - adres pocz�tkowy bloku
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT, NOMEM


Mnemonik:  REALLOC <nr> <size>
Parametry: <nr> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           <size> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opcode:    23
Opis:      realokuje blok pami�ci o numerze nr tak by mia� rozmiar size.
Efekt:     -
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT, NOMEM


Mnemonik:  DEALLOC <nr>
Parametry: <nr> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opcode:    24
Opis:      dealokuje blok pami�ci o numerze nr
Efekt:     -
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  CMPCNT <addr1> <addr2>
Parametry: <addr1> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           <addr2> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           u0 - liczba dwords
Opcode:    25
Opis:      por�wnuje <addr1> i <addr2> na przestrzeni u1 bajt�w
Efekt:     u0 - 0 = por�wnanie pomy�lne, !0 - niezgodno��
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  CPCNT <addr1> <addr2>
Parametry: <addr1> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           <addr2> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           u0 - liczba dwords
Opcode:    26
Opis:      przepisuje <addr2> do <addr1> na przestrzeni u1 bajt�w
Efekt:     -
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  ONFAIL <addr>
Parametry: <addr> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   27
Opis:	   skok w razie wyj�tku pod adres absolutny; anulowane po RET
           poni�ej obecnego poziomu wykonywania.
Efekt:	   -
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  NOFAIL
Parametry: -
Opcode:	   28
Opis:	   anulowanie ONFAIL na obecnym poziomie wykonywania
Efekt:	   -
Wyj�tki:   -


Mnemonik:  LOOP <addr>
Parametry: <addr> = IMMEDIATE, UREG, IMMPTR, UPTR
           s0 - licznik p�tli
Opcode:	   29
Opis:	   skok je�li s0 jest wi�ksze od zera pod adres absolutny,
           zmniejszenie s0 o jeden
Efekt:	   zmiana IP, zmiana s0
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  RAISE <nr>
Parametry: <nr> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   30
Opis:	   zg�oszenie wyj�tku <nr>
Efekt:	   zg�oszenie wyj�tku
Wyj�tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Jak zapewne zauwa�yli�cie, zarz�dzanie pami�ci� nie zosta�o umieszczone
w oddzielnym module i jest integraln� cz�ci� systemu. Jest to wyj�tek,
podyktowany wzgl�dami wydajno�ci oraz funkcjonalno�ci, nic oczywi�cie nie
przeszkadza w stworzeniu dowolnie wyrafinowanego systemu zarz�dzania
pami�ci� z wykorzystaniem syscalli.

Kolejna istotna uwaga - kilka os�b narzeka�o, i� ograniczony zakres
instrukcji RSIS nie pozwala na np. efektywne operacje graficzne czy generalnie
wyszukane dzia�ania na pami�ci. W RSIS - podobnie jak cho�by w C - podstawowy
j�zyk zapewnia tylko najprostsze operacje oraz konstrukcje pozwalaj�ce
sterowa� wykonywaniem etc. Natomiast wszystkie zaawansowane funkcje - jak
cho�by memfrob() lub podobne - s� funkcjami nie b�d�cymi elementami j�zyka,
a elementami bibliotek. W Argante mo�na napisa� cho�by modu� advgraph.c,
kt�ry b�dzie odpowiada� za z�o�one operacje na obiektach graficznych i
komunikacj� z kart�, ale nie nale�y oczekiwa� tego od RSIS (nie b�dzie
wi�c wydania Argante MMX ;). Cho� modu� obs�ugi grafiki nie jest w tej
chwili przewidywany, wiele funkcji pomocnych w obr�bce blok�w pami�ci
oraz danych tekstowych znajdzie si� w module advmem.c.



--------------------------------
 6. Wyj�tki niskopoziomowe (LLX)
--------------------------------

Wyj�tki zg�aszane przez polecenia (mog� by� obs�u�one ONFAIL):

#define ERROR_STACK_OVER                0x1

  Zglaszany przy przepelnieniu stosu podczas skoku - moze wystapic
  takze przy np. probie skoku do handlera wyjatku (a wiec w kazdej
  sytuacji). Proba obsluzenia jest przypadkiem beznadziejnym, chyba
  ze handler jest zadeklarowany gdzies wczesniej, nie na najwyzszym
  poziomie wykonywania.

#define ERROR_STACK_UNDER               0x2

  Niedomiar stosu. Zglaszany przy "przesadzonym" wywolaniu RET, gdy
  na stosie nie ma juz zadnych adresow. Obsluzony moze byc tylko
  rzecz jasna przez handler zadeklarowany na pierwotnym poziomie
  wykonywania, bo wszystkie wyzsze zostaly anulowane, jesli doszlismy
  do tego momentu ;)))

#define ERROR_OUTSIDE_CODE              0x3

  Instruction pointer wyszedl poza zaalokowana przestrzen kodu. Mozliwe
  zawsze, choc w rozsadnie naskrobanym programie nie powinno sie
  zdarzyc ;)

#define ERROR_OUTSIDE_REG               0x4

  Proba dostepu do rejestru spoza zakresu. Nie powinno miec miejsca,
  poniewaz kompilator kontroluje numery rejestrow - ale jesli ktos np.
  zmienil sobie config.h i przekompilowal Argante definiujac 5 rejestrow
  zamiast 16tu, a potem chcial uruchomic kod skompilowany wczesniej...
  No coz, nie trzeba tlumaczyc.

#define ERROR_BAD_PARAM                 0x5

  Jakis opcode ma nieprawidlowy parametr - mozliwe jesli np. mov jako
  pierwszy parametr ma wartosc immediate i w podobnych sytuacjach. Nie
  powinno sie zdarzac w kompilowanym kodzie. W pewnych sytuacjach
  taki wyjatek moze byc zglaszany przez REALLOC lub FREE - jesli numer
  bloku jest nieprawidlowy.

#define ERROR_BAD_INSTR                 0x6

  Nieprawidlowa instrukcja. Objaw "brain-damage" kompilatora, powaznej
  niekompatybilnosci lub czegos podobnego. Nie powinno sie zdarzac.

#define ERROR_OUTSIDE_MEM               0x7

  Proba dostepu do pamieci, ktora nie zostala zaalokowana - na ogol objaw
  poslugiwania sie wskaznikami do nieprzydzielonego adresu.

#define ERROR_PROTFAULT                 0x8

  W przypadku syscalli, wyjatek ten zglaszany jest gdy przekazany
  syscallowi adres, wskazujacy na bufor (parametr lub bufor wynikowy)
  nie moze byc zapisywany / odczytywany - a wiec jesli jest to
  nieprawidlowy adres lub strona jest chroniona przed zapisem (dla buforow
  wyjsciowych).

#define ERROR_TOOBIG                    0x9

  Proba zaalokowania bloku o rozmiarze przekraczajacym jednostkowy limit
  rozmiaru bloku.

#define ERROR_NOMODULE                  0xa

  Wywolanie syscalla, ktory nie jest obslugiwany przez zaden zaladowany
  obecnie modul (problem :).

#define ERROR_BAD_SYS_PARAM             0xb

  Syscallowi zostal przekazany nieprawidlowy parametr; ten komunikat
  zostal wyparty przez bardziej opisowe kody (zachowany dla celow
  kompatybilnosci ;).

#define ERROR_ACL_PROBLEM               0xc

  Blad systemu HAC - nie jest mozliwe zaladowanie zestawu regul
  kontrolujacych dostep. Oznacza to, ze kazda proba korzystania z
  podsystemu HAC skonczy sie zgloszeniem tego wyjatku, oraz prawdopodobnie
  dalszymi wyjatkami (dotyczacymi odmowy dostepu do okreslonego obiektu).

#define ERROR_NOPERM                    0xd

  Odmowa dostepu do okreslonego syscalla przez HAC.

#define ERROR_NOMEM                     0xe

  Brak wolnych blokow / brak wolnej pamieci. Mozliwe przy ALLOC/REALLOC.

#define ERROR_DEADLOCK                  0xf

  "Sytuacja bez wyjscia" - na przyklad brak dostepu do wymaganego zasobu
  systemowego.

#define ERROR_NOOBJECT                  0x10

  Obiekt nie istnieje w hierarchii SVFS (blad przy operacjach na plikach).

#define ERROR_FSERROR                   0x11

  Ogolny blad modulu fs - na przyklad niemoznosc dokonania okreslonej
  operacji mimo, iz powinno byc to mozliwe (generalnie: niepowodzenie
  syscalla rzeczywistego systemu).

#define ERROR_FS_BAD_PATH               0x12

  Sciezka jest nieprawidlowa - uzyto relatywnej sciezki gdy dla procesu
  nie byl zdefiniowany aktualny katalog roboczy, lub sciezka / jej element
  byl zbyt dlugi / zawieral nieprawidlowe znaki.

#define ERROR_FS_OPEN_ERROR             0x13

  Blad otwierania pliku - np. plik nie istnieje. Klasyczna sytuacja -
  plik zostaje usuniety w trakcie IOWAIT, albo po prostu nigdy nie
  istnial.

#define ERROR_FS_BAD_OPEN_MODE          0x14

  Tryb dostepu do pliku, o ktory sie upomniano, jest nieprawidlowy.

#define ERROR_FS_CREATE_ERROR           0x15

  Nie mozna utworzyc pliku (np. plik istnieje).


#define ERROR_FS_BAD_VFD                0x16

  Przy operacjach I/O w obrebie otwartego pliku, podano nieprawidlowy
  VFD - np. niedozwolona wartosc lub numer, ktory nie jest wykorzystywany.

#define ERROR_FS_NOSEEK                 0x17

  Plik jest otwarty w trybie append-only, podczas gdy wykonano operacje
  typu seek.

#define ERROR_FS_EXISTS                 0x18

  Przy operacji typu rename, plik docelowy istnial.

#define ERROR_FS_NOFILE                 0x19

  Obiekt nie istnieje lub nie jest plikiem. Mozliwe w przypadku operacji
  przenoszenia, zmiany nazwy (nie istnieje), otwierania (nie jest
  plikiem). Dotyczy takze funkcji LIST_DIR (katalog nie istnieje).

#define ERROR_FS_NODIRENT               0x1a

  Podany numer pozycji w obrebie katalogu jest nieprawidlowy.

#define ERROR_RESULT_TOOLONG            0x1b

  Rezultat jest wiekszy, niz bufor przekazany syscallowi, zwrocenie
  wartosci nie bylo mozliwe.

Kto jeszcze nie kumie, jak dzialaja wyjatki, niech zerknie na
compiler/examples/error2.agt.

...

Przyj�t� polityk� jest informowanie (u�ywaj�c funkcji non_fatal())
programu za pomoc� zg�aszanych wyj�tk�w o sytuacjach nietypowych /
alarmowych. Tak wi�c syscall sprawdzaj�cy istnienie pliku w razie jego
nieobecno�ci nie powinien zg�asza� wyj�tku. Natomiast syscall s�u��cy
do otworzenia pliku w tej sytuacji powinien go zg�osi�.

W chwili wywo�ania handlera ONFAIL w u0 znajduje si� numer wyj�tku.
Po wykonaniu ret przywracana jest pierwotna zawarto�� u0. O ewentualne
zapisanie reszty modyfikowanych rejestr�w handler powinien zatroszczy� si�
oczywi�cie sam.

Jak dzia�aj� wyj�tki? Na ka�dym poziomie wykonywania, mo�emy zg�osi�
handler. Np. jeste�my w g��wnym kodzie (stack ptr == 0), i za pomoc� ONFAIL
deklarujemy handler wyj�tk�w. Nast�pnie wywo�ujemy funkcj� lokaln�, kt�ra
ma w�asny ONFAIL - wtedy wyj�tki nie b�d� "przekazywane" do g��wnego
poziomu wykonywania. Je�li potem wr�cimy z funkcji lokalnej i wywo�amy
inn�, bez oddzielnego ONFAIL, w razie wyst�pienia wyj�tku funkcja (i
wszystkie p�niej wywo�ane) zostan� "puszczone w niebyt", i program powr�ci
do g��wnego w�tku, wywo�uj�c to, co zadeklarowano jako ONFAIL na tym
poziomie wykonywania.


-------------------------------------
 7. HAC - system kontroli przywilej�w
-------------------------------------


Stworzony jest zunifikowany mechanizm zarz�dzania przywilejami (HAC).
Przyk�adowy wpis w pliku access.set, kt�ry jest plikiem konfiguracyjnym
podsystemu:

12345:00000     fs/ftp/uzytkownicy      fs/fops/new/dir         allow
|               |                       |                       |
| +-------------+                       |      znaczenie wpisu _|
| | +-----------------------------------+     - allow lub deny
| | |
| | +-- hierarchiczny identyfikator rodzaju dost�pu: przestrze� uprawnie�
| |     to 'fs', ga��� operacji na plikach (fops), rodzaj operacji: tworzenie
| |     obiekt�w (new), rodzaj obiektu: katalog (dir). Jest to konwencja
| |     sugerowana, tak jak m�wi�em kernel nie odpowiada za autoryzacj�.
| |     Robi� to modu�y, przekazuj�c dane funkcji is_permitted().
| |
| +---- hierarchiczny identyfikator zasobu; w tym przypadku opisana jest
|       przestrze� obiektu, przestrze� systemu plik�w (pliki ftp),
|       oraz konkretny katalog.
|
+------ przynale�no�� do grupy; warto�� '0' oznacza, i� regu�a ma charakter
        "generic" i odnosi si� do wszystkich grup; warto�� po znaku ':'
        oznacza podgrup�. Tu zgodno��, w przypadku regu� specyfikuj�cych
        niezerow� grup�, musi by� pe�na.

Wpisy s� honorowane w kolejno�ci wyst�powania w pliku konfiguracyjnym.
Dlatego bardziej specyficzne wpisy (np. zawieraj�ce odmow� dost�pu do zasobu
dla danej podgrupy) powinny znale�� si� przed og�lniejszymi.

UWAGA: Je�li identyfikator operacji to w pliku konfiguracyjnym to np.
'fs/fops', oznacza to, i� osobnik podpadaj�cy pod pozosta�e kryteria,
a chc�cy dost�pu np. do 'fs/fops/new/file/text', otrzyma te uprawnienia.
Oczywi�cie nie dzia�a to w drug� stron� i wpis 'fs/fops/new/file/text'
nie implikuje dost�pu do ca�ej hierarchii 'fs/fops'. U�ywanie '/'
jako separator�w jest konieczne - np wpis 'fs_ops' nie oznacza dost�pu
do obiektu 'fs_ops_new_file'.

HAC narzuca regu�� precyzowania operacji w stron� u�cislania ich typu
obiektu, na kt�rym operacja jest wykonywana. Tak wi�c _ZAWSZE_ poprawnym
hierarchicznie zapisem jest np.:

    +- wdops ---- cwd
    |    +------- pwd
    |
    +- setup ---- ...
    |
fs -+- fops --+-- create -- file ----------+-- binary
              |      +----- directory      |
              |                            +-- text
              +-- delete -- file
              |      +----- directory
              |
              +-- read ---- directory
                    +------ file

Natomiast NIEPOPRAWNYM zapisem jest na przyk�ad fs/fops/file/delete,
fs/fops/file/create, etc. Cho� w wielu sytuacjach na pierwszy rzut oka mo�e si�
to wyda� nielogiczne, to jednak drugi zapis praktycznie uniemo�liwi�by
generalizowanie regu� (np. danie uprawnie� do tworzenia obiekt�w w
danej cz�ci filesystemu to wpisanie fs/fops/create, podczas gdy w drugiej
notacji oznacza�oby to wiele wpis�w).

Aby chroni� si� przed pr�bami wprowadzenia w b��d modu��w obs�uguj�cych
filesystem, system autoryzacji odmawia dost�pu do obiekt�w zawieraj�cych
ci�g "/..". Ich eliminacj� powinien zaj�� si� modu� (i zajmuje si�).

W przypadku podsystem�w, w przypadku kt�rych nie da si� okre�li� zasobu,
b�d� jego okre�lanie by�oby dublowaniem rodzaju dost�pu (np. modu�
wy�wietlaj�cy tekst na wirtualnej konsoli - okre�lenie rodzaju operacji
jest tu wystarczaj�ce), jako zas�b nale�y poda� "none".

Testowanie regu�ek przed ich aktualizacj� mo�e odby� si� z u�yciem
do��czonego programu 'actest' (w katalogu tools), kt�ry zapewnia
do�� solidn� diagnostyk� HAC. Aktualizacja regu�ek - polecenie '^' na
konsoli zarz�dzaj�cej (patrz dalej).

Je�li nie wiesz jeszcze nic o modu�ach, mo�esz wr�ci� tu p�niej. Poni�ej
znajduje si� interfejs HAC dla modu��w:


Z punktu widzenia autora modu�u, najwygodniejszym interfejsem do
kontroli dost�pu jest makro VALIDATE() z pliku include/acman.h.
Makro akceptuje trzy parametry: numer procesora, identyfikator zasobu
i identyfikator typu dost�pu.

Na przyk�ad:

  VALIDATE(c,"net/tcp/destination/10.0.0.1/1234","net/connect");

W przypadku, gdy dost�p jest mo�liwy, makro nie odniesie efektu. W razie
odmowy dost�pu, makro zg�osi wyj�tek NOPERM z opisem sytuacji i wyjdzie
z funkcji syscall_handler(), z kt�rej powinno by� wywo�ane.

W celu obs�ugi w bardziej finezyjny spos�b, mo�na korzysta� z funkcji
is_permitted(), akceptuj�cej parametry zgodne z parametrami makra
VALIDATE(), ale zwracaj�cego warto�ci 0 (odmowa) lub 1 (dost�p mo�liwy).
Nie jest zg�aszany wyj�tek ani nie ma miejsca return z funkcji. Dla
�cis�o�ci, makro VALIDATE() jest zbudowane w nast�puj�cy spos�b jako
wrapper do funkcji is_permitted():

#define VALIDATE(cp,res,act) { \
    char errbuf[512]; \
    if (!is_permitted(cp,res,act)) { \
      if (!cpu[cp].fail_safe) \
        snprintf(errbuf,200,"permision denied [%d:%d] act='%s' obj='%s'", \
                 cpu[cp].current_domain,cpu[(cp)].domain_uid,act,res); \
      non_fatal(ERROR_NOPERM,errbuf,(cp)); \
      return; \
    } \
  }

Modu� powinien wysy�a� mo�liwie jak najbardziej dospecyfikowane zapytanie,
zawieraj�ce komplet danych niezb�dnych do weryfikacji uprawnie�. Modu�
obs�uguj�cy grafik� nie powinien wi�c pyta� o 'graph' tylko o
'graph/control/setmode' i o zasob 'graph/res/640/480/16bpp'. Podobnie w
pliku konfiguracyjnym access.hac nale�y mo�liwie dok�adnie zaw�a�
regu�y.


--------------------------------------
 8. SVFS - architektura systemu plik�w
--------------------------------------

System plik�w definiowany jest w pliku conf/fsconv.dat. Zawiera on
oddzielony spacjami model mapowania wirtualnego systemu plik�w na system
rzeczywisty. Regu�y doboru s� podobne jak w przypadku HAC:

fs/ftp/test1            /Argante/fs/inny_katalog
fs/ftp                  /Argante/fs/serwer_ftp

HAC kontroluje dost�p na poziomie wirtualnych katalog�w. Powy�sze wpisy
oznaczaj�, �e katalog fs/ftp/test1 jest mapowany w inne miejsce, ni� katalog
fs/ftp. Je�li jaki� proces b�dzie posiada� wpis HAC pozwalaj�cy na
operacj� typu fs/create/directory na obiekcie fs/ftp, b�dzie posiada� dost�p
do obu katalog�w (zgodnie z za�o�eniami HAC, o ile nie zostanie to
wcze�niej wykluczone). Tworz�c katalog fs/ftp/nope, rzeczywisty wpis
powstanie w miejscu /Argante/fs/serwer_ftp/nope. Natomiast ta sama operacja
dla fs/ftp/test1/nope, zaowocuje plikiem /Argante/fs/inny_katalog/nope.
Natomiast pr�ba dost�pu np do obiektu fs/ftp/../nope sko�czy si�
niepowodzeniem - modu� systemu plik�w rozpozna j� jako dost�p do obiektu
fs/nope, tymczasem taki wpis nie istnieje w hierarchii SVFS.

Filozofia systemu plik�w Argante zak�ada jednocze�nie niezale�n� od
rzeczywistego systemu kontrol� dost�pu do zasob�w i ochron� systemu
macierzystego, a r�wnocze�nie mo�liwo�� integracji systemu plik�w SVFS
z obiektami rzeczywistego systemu plik�w.

System SVFS jest mo�liwie uproszczonym, lecz w pe�ni funkcjonalnym
podzbiorem operacji na systemu plik�w. W pierwotnej wersji nie posiada
wsparcia dla tworzenia symlink�w lub hardlink�w, aczkolwiek wspiera te
istniej�ce na poziomie systemu rzeczywistego.

W��czanie istotnych zasob�w / katalog�w systemowych bezpo�rednio w
hierarchi� SVFS jest mo�liwe (np. udost�pnienie katalogu /etc), ale
zdecydowanie odradzane.


------------------------------------------------------------
 9. IPC/rIPC - komunikacja mi�dzyprocesowa <w przygotowaniu>
------------------------------------------------------------

Je�li chcia�by� zaj�� si� wdro�eniem systemu IPC - daj zna�!

Modu� ten nie jest jeszcze wdro�ony - poni�ej znajduje si� wczesny draft:

W komunikacji mi�dzyprocesowej warunkiem jest to, i� dwa procesy chc�ce
otworzy� mi�dzy sob� sesj� [r]IPC, musz� przynale�e� przynajmniej do jednej
wsp�lnej grupy (lub podgrupy, je�li ta zosta�a ustawiona), a tak�e musz�
wyrazi� ch�� nawi�zania sesji. Oczywi�cie mo�liwa jest tak�e kontrola
mo�liwo�ci korzystania z mechanizmu IPC w og�le za pomoc� HAC.

Ka�dy proces, kt�ry chce korzysta� z IPC, musi zarejestrowa� sw�j numer
identyfikacyjny na potrzeby komunikacji i zna� numer identyfikacyjny
drugiej strony (mo�liwe jest nadanie dw�m lub wi�cej procesom tego samego
numeru w celu implementacji dystrybucji sesji IPC).

Zasad� funkcjonowania IPC jest potwierdzanie ka�dego dzia�ania - tak wi�c
je�li jedna strona, po nawi�zaniu sesji IPC (handshake) za�yczy sobie
skopiowania okre�lonego obszaru pami�ci z przestrzeni jednego procesu
do przestrzeni drugiego procesu, druga strona musi odebra� request i
wys�a� potwierdzenie do modu�u IPC.

Dzi�ki statycznym parametrom proces�w, mo�liwa jest jednoznaczna identyfikacja
programu zg�aszaj�cego request i jednoznaczne okre�lenie odbiorcy, dane te
s� jawne dla obu stron (udost�pniane przez modu�). Proces wygl�da tak samo,
niezale�nie od tego, czy procesy dzia�aj� w obr�bie jednego systemu fizycznego,
czy system�w rozproszonych w sieci. W tym drugim przypadku po��czenie
system�w IPC na dw�ch hostach musi zosta� zainicjowane przez operatora lub
z poziomu skrypt�w nadzorczych.

Oczekiwana funkcjonalno�� IPC (wszystko z obustronnym potwierdzeniem
i obustronnym zg�oszeniem gotowo�ci odbioru / nadawania danych:

- Rejestracja w systemie (ustawienie IPCREG):

  Syscalle: IPC_REGISTER

- zestawianie po��cze� strumieniowych (odpowiednik unnamed pipes)
  [request nawi�zania sesji mo�e by� wys�any do jednego procesu,
  do wszystkich proces�w o danym IPCREG, do wszystkich proces�w na danym
  ho�cie albo do wszystkich host�w w og�le - broadcast; te mog�, ale
  nie musz� go odebra�, wywo�uj�c odpowiedni syscall; warto zrobi�
  niewielk� kolejk� request�w IPC]. Po IPC_CHECK_QUEUE a przed IPC_REJECT
  lub IPC_ACCEPT ostatnia pozycja kolejki powinna by� "zamra�ana",

  Syscalle: IPC_STREAM_ASK, IPC_CHECK_QUEUE, IPC_REJECT, IPC_ACCEPT,
            IPC_SEND_DATA, IPC_RECEIVE_DATA, IPC_CLOSE, IPC_STATUS

- zestawianie po��cze� blokowych (odpowiednik wsp�dzielenia pami�ci).
  [j.w.]

  Syscalle: IPC_BLOCK_ASK [cz�� j.w.] IPC_WRITE_BLOCK, IPC_RECEIVE_BLOCK

- wysy�anie komunikat�w (2 x dword);
  [j.w.]

  Syscalle: IPC_SEND_MSG

Co z rIPC? Pomys� jest r�wnocze�nie prosty i bardzo przebieg�y. Ot�
w�a�ciwie nic. Modu� rIPC rozszerza funkcjonalno�� modu�u IPC pozwalaj�c
linkowa� ze sob� predefionowane serwery. Struktura linkowania mo�e by�
dowolna, ka�dy serwer posiada odpowiedni identyfikator w hierarchii (numery 
te nie s� u�ywane do cel�w innych ni� unikanie p�tli). Ka�dy do��czony
serwer informuje inne serwery, jakie procesy posiadaj� jakie numery IPCREG,
i zapewnia przesy�anie zapyta� IPCREG w obr�bie po��cze� (requesty s�
po nadaniu identyfikatora broadcastowane).

Co z tego wynika?

- mo�liwo�� "rozproszenia" aplikacji mi�dzy wiele maszyn bez konieczno�ci
  wprowadzania jakichkolwiek zmian w kodzie,

- mo�liwo�� uruchomienia w kilku punktach sieci proces�w posiadaj�cych
  ten sam identyfikator IPCREG; zapytania do nich b�d� wtedy rozdzielane
  (load balancing / przetwarzanie rozproszone) - round robin

- mo�liwo�� tworzenia klas funkcjonalnie to�samych maszyn w r�nych
  punktach sieci, kt�re s� w stanie realizowa� zadania r�wnocze�nie,

- mo�liwo�� tworzenia redundantnej struktury po��cze�,

Jak ju� m�wi�em, wynikaj� z tego niespotykane mo�liwo�ci tworzenia
samoorganizuj�cych, adaptuj�cych si� do sytuacji, potrzeb i wymaga� w
spos�b nie wymagaj�cy interwencji u�ytkownika system�w clustrowych. Argante
mo�e stanowi� platform� zarz�dzaj�co-kontroln� takich clustr�w. Podobnych
mo�liwo�ci tworzenia clustr�w nie oferuj� chyba �adne inne rozwi�zania.

Aha, dystrybucj� request�w mi�dzy to�same procesy powinien wspiera� te�
"czysty" IPC ;>


-----------------------------------
10. Skrypty i zarz�dzanie konsolowe
-----------------------------------

Ilekro� pisz� "skrypty nadzorcze", "operator", nie mam na my�li specjalnego,
uprzywilejowanego konta administratora, tylko obs�ugiwan� z poziomu
kernel-space konsol� zarz�dzaj�c�. W chwili startu systemu, wykonywane s�
skrypty startowe (kt�re mog� mi�dzy innymi wczytywa� modu�y i nawi�zywa�
po��czenia sesji rIPC, a tak�e wczytywa� procesy).

Zarz�dzanie prac� systemu wirtualnego nie odbywa si� z poziomu zada�
wykonywanych wewn�trz systemu.

Konsola Argante oferuje do�� prosty zestaw polece�, kt�re s�u�� g��wnie do
uruchamiania proces�w i zarz�dzania bibliotekami. Oto one:

?               - pomoc

!               - statystyka systemowa

$fn             - wczytanie obrazu binarnego z pliku fn i uruchomienie go
                  na pierwszym wolnym VCPU

%fn		- jak wy�ej, wczytuje zadanie w trybie RESPAWN (b�dzie ono
		  uruchomione ponownie je�li dojdzie do zako�czenia pracy
	          danego procesu poleceniem innym ni� HALT)

                  UWAGA: ten tryb s�u�y do wykonywania program�w, kt�re
	          powinny dzia�a� ca�y czas; generalnie jednak, nale�y
		  stawia� nacisk na prawid�owe funkcjonowanie procesu
		  i obs�ug� wyj�tk�w w ka�dej sytuacji, a tak�e na tworzenie
		  redundantnych proces�w w hierarchii IPC, a t� opcj� traktowa�
	          tylko jako rozwi�zanie pomocnicze. Jako zabezpieczenie
	          przed nadu�yciem tego trybu, istnieje zmienna
		  MIN_CYCLES_TO_RESPAWN definiowana w pliku config.h, kt�ra
		  okre�la minimaln� ilo�� cykli pracy przed wyst�pieniem
		  sytuacji prowadz�cej do �mierci procesu (32). Je�li b��d
		  wyst�pi wcze�niej, program nie zostanie uruchomiony ponownie.

>fn             - wczytanie biblioteki z pliku fn do wolnego slotu

<id             - skasowanie biblioteki w slocie 'id'

#               - wy�wietlenie listy bibliotek wraz ze statystyk�
                  (obs�ugiwane syscalle, ilo�� wywo�a�)

@fn             - uruchomienie skryptu konsolowego

-nn             - zabicie procesu na VCPU numer nn

=nn             - wy�wietlenie statystyki dla procesu na VCPU numer nn
.               - halt systemu

*nn             - wykonanie nn takt�w systemu bez sprawdzania wej�cia
                  na konsoli zarz�dzaj�cej; przydatne w skryptach.

:xx             - wywo�anie subshella i wykonanie polecenia "xx"

|xx             - "nic" - komentarz w skrypcie.

^               - ponowne wczytanie tablicy HAC

w nn tmout      - czekaj na zako�czenie procesu nn przez tmout sekund

Dost�pne s� tak�e inne polecenia, s�u��ce do debugowania, opisane w
dalszych cz�ciach dokumentu.

Jak zapewne zauwa�y�e�, konsola jest elementem sk�adowym systemu - w tym
sensie, �e zarz�dzanie mo�e odbywa� si� bezpo�rednio po zbootowaniu.
Oczywi�cie jest to tylko wygodna opcja, dost�p do sesji Argante mo�e by�
realizowany w inny spos�b (patrz polecenia agtback i agtses). W obecnej
chwili nie uwa�ali�my za istotne oddzielanie kodu konsoli od samego systemu,
jako �e nie jest to w �adnym wypadku "kosztowne" rozwi�zanie (nie obni�a
wydajno�ci), a zapewnia �atwo�� zarz�dzania w ka�dej sytuacji. By� mo�e 
system i konsola zostan� rozdzielone w przysz�o�ci.

Skrypty maj� sk�adni� analogiczn� do polece� konsolowych. Po uruchomieniu
systemu wykonywany jest skrypt argboot.scr, lub inny skrypt, je�li
podany zostanie w linii polece� (je�li podany by� tak�e drugi parametr
z linii polece�, ten w�a�nie katalog zostanie przyj�ty jako pocz�tkowy
katalog wykonywania - z plikami konfiguracyjnymi, filesystemem etc - jak
d�ugo plik config.h nie definiuje �cie�ek absolutnych, lecz relatywne).
Przyk�ad skryptu:

--
|
| Argente system test script
| (C) 2000 Michal Zalewski
|

~Loading system modules...
>modules/display.so
>modules/access.so
>modules/fs.so
~
~  ***************************
~  * Testowy Skrypt Lcamtufa *
~  ***************************
~
:compiler/agtc compiler/hello.agt
$compiler/hello.img
w 0 10
~Koniec pracy ;>
.
--

Konsol� domy�ln� jest stdin procesu 'argante' w chwili odpalenia. Oczywi�cie,
mo�e nie by� to dzia�anie oczekiwane przez administratora. Mo�liwe jest
wi�c uruchomienie argante w tle w nast�puj�cy spos�b:

tools/agtback sciezka-do-argante [ nazwa-skryptu ]

Nale�y zwr�ci� uwag� na to, by w obecnym katalogu istnia�o odpowiednie
�rodowisko uruchomieniowe - modu�y, skrypty startowe w odpowiednich
katalogach.

Praca na konsoli sesji pozostawionej w tle jest mo�liwa z wykorzystaniem
polecenia agtses. Jako parametr nale�y poda� mu numer procesu Argante.


-------------------------------------
11. Pos�ugiwanie si� RSIS-assemblerem
-------------------------------------

Tak wygl�da przyk�adowy plik �r�d�owy, kt�ry wy�wietla liczby od 10 do 0
i przy okazji troch� gada:

--
!SIGNATURE      "program testowy lcamtufa"

.DATA

:Enter

        "\n"

:Tekst

        " Hello world\n"

:Umieraj

        "Aghrrr... Umieram.\n\n"

.CODE

  mov u0,:Enter
  mov u1,^Enter
  syscall $IO_PUTSTRING

  mov s0,0xa

:Dalej

  mov u0,s0
  syscall $IO_PUTINT

  mov u0,:Tekst
  mov u1,^Tekst
  syscall $IO_PUTSTRING

  twait 500000

  loop :Dalej

  mov u0,:Umieraj
  mov u1,^Umieraj
  syscall $IO_PUTSTRING

  halt

.END
--

R�ne przyk�ady (pliki *.agt) mo�na znale�� w podkatalogu compiler/examples -
opr�cz podobnego "hello world" jest tam tak�e np. przyk�ad obs�ugi
wyj�tk�w (error.agt) czy systemu plik�w (fs.agt). Sam j�zyk AGT ma nast�puj�c�
sk�adni�:

.DATA, .CODE - definicje kolejnych segmentow (.data jest opcjonalne).
	       Dzi�ki bulbie, mo�esz prze��cza� si� mi�dzy segmentami
	       kiedy chcesz :)

.END         - ko�czy segment kodu

:xxx         - zar�wno w segmencie kodu, jak i w segmencie danych, oznacza
               symboliczn� nazw� s�u��c� do wskazywania obiektu znajduj�cego
               si� w nast�pnej linii; musi wyst�powa� w oddzielnej linii,
               w segmencie danych wszystkie obiekty musz� mie� nazw�.

               Dane mo�na umieszcza� w formacie:

                "xxxx" - ci�g znak�w
                123    - warto�� liczbowa ca�kowita (32-bitowa)
                123.0  - warto�� liczbowa zmiennoprzecinkowa (j.w.)
                0x123  - warto�� hexadecymalna 32bit

                NN repeat 123 - blok 123 powt�rze� warto�ci NN
                                (zmiennoprzecinkowa lub ca�kowita)

                block 100 - nast�pne 100 linii b�dzie zawiera�o warto�ci
                            (dwords) do wpisania do struktury.

                Wywo�ania do symboli przekazywanych jako parametry, musz�
                mie� nast�puj�c� posta�: ':Symbol'. Inne mo�liwo�ci to
                '^Symbol' - zwraca d�ugo�� obiektu w bajtach, przydatne dla
                ci�g�w tekstowych; '%Symbol' - zwraca d�ugo�� w dwords.

!xxx         - dyrektywa kompilacji, okre�la parametry procesu. Dopuszczalne:

               !DOMAINS x x x     - lista grup wykonywania
               !PRIORITY x        - priorytet programu
               !IPCREG x          - pocz�tkowy identyfikator IPC
               !INITDOMAIN x      - pocz�tkowa grupa wykonywania
               !SIGNATURE x       - sygnatura kodu (opis, autor)
               !INITUID x	  - pocz�tkowy identyfikator podgrupy

Dopuszczalne jest identyfikowanie syscalli na podstawie ich symbolu, o ile
ten jest znany kompilatorowi. Lista syscalli znajduje sie w syscall.h w
katalogu modules/. Nazwa syscalla musi byc poprzedzona znakiem $ - na
przyk�ad 'syscall $io_putstring' (uwaga - pomijamy przedrostek syscall_).
Tak samo mo�na odwo�ywa� si� do numer�w wyj�tk�w - ich nazwy znajduj� si�
w pliku include/exception.h, tu pomijamy error_.

Aha - priorytet '1' jest domy�ln� warto�ci�, aczkolwiek nie jest zbyt
rozs�dny. Sugeruj� priorytety w granicach 10-10000, poniewa� wtedy w ka�dym
cyklu wykonywane jest wi�cej polece� maszynowych, a obr�bka ich za jednym
zamachem jest efektywniejsza ni� kolejne skoki.

Rejestry nale�y podawa� w formacie 'xNN', gdzie NN to numer rejestru,
a x to odpowiednio 'u' (ureg), 's' (sreg), 'f' (freg). I tak na przyk�ad 'u0'
to ureg[0].

Poprzedzenie warto�ci liczbowej, symbolu lub rejestru 'u' znakiem '*'
oznacza, �e chodzi o warto�� znajduj�c� si� pod danym adresem. Na przyk�ad:

mov u0,*:Test

Spowoduje wpisanie do rejestru u0 warto�ci znajduj�cej si� pod adresem Test,
podczas gdy:

mov u0,:Test

Spowoduje wpisanie adresu wskazywanego przez identyfikator 'Test' do rejestru
u0.

Kompilator, przynajmniej w obecnej wersji, nie b�dzie wspiera� arytmetyki
na poziomie kompilacji. System nie wspiera tak�e wielu rozdzielnych
blok�w pami�ci przydzielanych w chwili �adowania binarki.

Kompilator uruchamia si� wpisuj�c "compiler/agtc plik.agt". Efektem b�dzie
plik binarny plik.img, kt�ry mo�na za�adowa� poleceniem $ w konsoli
zarz�dzaj�cej.


--------------------------------------------------------
12. Pos�ugiwanie si� translatorem AHLL <w przygotowaniu>
--------------------------------------------------------

0) Wprowadzenie do AHLL
~~~~~~~~~~~~~~~~~~~~~~~

Generalnie, konwencje AHLL wahaj� si� mi�dzy C (w kwestiach sk�adni)
do Ady (filozofia wska�nik�w, w�a�ciwo�ci, kontroli sk�adni). Oczywi�cie
si�� rzeczy ten j�zyk jest tylko "marnym" podzbiorem j�zyk�w macierzystych,
aczkolwiek w pe�ni wystarcza do pisania nawet ca�kiem z�o�onych projekt�w
- przynajmniej tak� mam nadziej� i takie s� za�o�enia.

Je�li znasz Pascala i C, a tak�e masz poj�cie o Argante, powiniene� nie mie�
�adnych problem�w ze zrozumieniem filozofii AHLL.

Oczywi�cie j�zyk ten powinien by� du�o bardziej urozmaicony - ale tak si�
sk�ada, �e system ten powstaje od podstaw, i musz� stawia� sobie w miar�
realne cele, kt�re stopniowo realizuj�. My�l�, �e ten j�zyk i jego specyfikacja
powinna by� dalej rozwijana, ja po prostu stwarzam podstawy.


1) Definicje globalne
~~~~~~~~~~~~~~~~~~~~~

Defniowanie zmiennych wygl�da generalnie w nast�puj�cy spos�b:

nazwa_zmiennej	:  nazwa_typu [ (parametry) ]  [ := Wartosc_Poczatkowa ] ;

W przypadku typ�w z�o�onych (tablic lub struktur) wartosc_poczatkowa mo�e
by� zbiorem warto�ci uj�tym w nawiasy klamrowe, na przyk�ad:

    moja_zmienna : moj_typ_tablicowy := { 0, 1, 2, 3 };

Nazwa zmiennej, jak i wszystkie inne symbole AHLL, jest case-insensitive,
i podlega tradycyjnym konwencjom - mo�e sk�ada� si� ze znak�w alfanumerycznych
oraz podkre�lenia ("_"), musi zaczyna� si� od litery. Maksymalna d�ugo��
identyfikatora to 64 znaki.

Nazwa typu podlega takim samym zasadom. Typ ten musi by� typem
predefiniowanym lub jednym ze zadeklarowanych wcze�niej typ�w. Je�li
podany jest inicjalizator, musi by� on warto�ci� daj�c� si� okre�li� w
chwili kompilacji, i musi posiada� ten sam typ oraz rozmiar, co inicjalizowany
obiekt.

Niekt�re typy pozwalaj� podawa� parametry w chwili inicjalizacji. Parametry
te mog� okre�la� pocz�tkowe i/lub ko�cowe brzegi przedzia�u warto�ci danego
typu lub rozmiar tablicy. Wywo�anie wygl�da wtedy nast�puj�co:

  moja_zmienna : moj_typ_tablicowy(100);

Je�li konieczne jest, by jako parametr przy inicjalizacji typu przekazano
warto�� okre�laj�c� w jaki� spos�b inicjalizator, mo�e odby� si� to przez
skorzystanie ze specjalnego symbolu "Self" posiadaj�cego nast�puj�ce 
"w�asno�ci" dla typ�w prostych:

Self'Dword_Length	- okre�la d�ugo�� inicjalizatora w dwords
Self'Byte_Length	- okre�la d�ugo�� inicjalizatora w bajtach
Self			- warto�� inicjalizatora.

Oraz nast�puj�ce dla tablic / typ�w strukturalnych:

Self'Count		- ilo�� element�w inicjalizatora
Self[n]'Dword_Length    - d�ugo�� n-tego pola inicjalizatora w dwords
Self[n]'Byte_Length     - d�ugo�� n-tego pola inicjalizatora w bajtach
Self[n]			- warto�� n-tego pola inicjalizatora

Uwaga: mo�liwe jest tylko bezpo�rednie inicjalizowanie element�w tablicy
lub struktury, bez "zaz�biania" inicjalizacji. A wi�c poprawn� konstrukcj�
jest:

 [ ...definicja typu Ala, ktory jest struktura zawierajaca m ciagi znakow... ]
 [ ...definicja typu Kot, ktory jest tablica n obiektow typu Ala... ]

 ala_1   : Ala (Self'Count) := { "kot", "ma", "Ale" };
 ala_2   : Ala (Self'Count) := { "Ala", "ma", "kota" };
 moj_kot : Kot (Self'Count) := { ala_1, ala_2, ala_1 };

Je�li jako cz�� inicjalizatora trzeba przekaza� warto�� okre�laj�c� w jaki�
spos�b inny jego element, mo�na zrobi� to w nast�puj�cy spos�b:

  ala : BoundedChunk := { "tekst", Self[1].Byte_Length };
 
Natomiast nieprawid�owe by�oby rozwi�zanie d���ce do po��czenia inicjalizacji
tablicy (Kot) z inicjalizacj� struktury (Ala):

  // Blad!
  moj_kot : Kot (Ini'Count) := { { "kot", "ma", "Ale" } , { "Ala", ... } ... };

Dlaczego? No c�, cho�by w celu unikni�cia problem�w z to�samo�ci� element�w,
z przekazywaniem ewentualnych parametr�w do inicjalizator�w struktur w
tablicy etc.

Predefiniowane typy to:

  unsigned	- unsigned integer, 4 bajty

  signed	- signed integer, 4 bajty

  float		- float, 4 bajty

  proc_address	- specjalny typ s�u��cy do przechowywania adresu procedur;
                  typ ten nie mo�e by� ograniczony z wykorzystaniem
                  modyfikatora range

Semantyka AHLL zak�ada, i� nie jest mo�liwe przeprowadzanie przypisa� mi�dzy
r�nymi typami. Dopuszczalne jest przeprowadzanie konwersji za pomoc� dyrektywy
Convert(zmienna) - pod warunkiem, i� typy s� typami prostymi.

Nale�y liczy� si� z tym, �e je�li warto�� konwertowana wykracza poza granice
warto�ci dopuszczalnych dla warto�ci docelowej, dojdzie do zg�oszenia
wyj�tku (patrz sekcja 5).

Definiuj�c zmienne, mo�na pos�u�y� si� nast�puj�cymi modyfikatorami przed
nazw� typu:

  pointer to	- warto�� jest wska�nikiem. Nie mo�e by� u�ywana do czasu,
                  gdy nie zostanie zaalokowana (lub nie zacznie wskazywa�
                  na inny obiekt danego typu) ani po tym, jak zostanie
                  zwolniona.

  addressable   - obiekt mo�e by� wskazywany przez warto�ci wska�nikowe,
                  program mo�e mie� dost�p do atrybutu 'Address.

Wykorzystywanie tych typ�w w celu uzyskania tablic, struktur itp, zosta�o
opisane w sekcji 3.


2) Prekompilacja
~~~~~~~~~~~~~~~~

Sta�e mo�na wprowadzi� do programu korzystaj�c z opcji prekompilatora:

#define nazwa_stalej wartosc

Mo�liwe jest tak�e w��czanie kodu z innych plik�w do zasadniczego kodu,
u�ywaj�c dyrektywy:

#include "nazwa_pliku"

Dopuszczalne komenatrze (bez nestingu) s� w konwencji C: // lub /* */

Kompilator wspiera tak�e przekazywanie kodu dla agtc - przez
dyrektyw� #compiler. Na przyk�ad:

#compiler !DOMAINS 1 2

Na razie kompilator nie obs�uguje �adnej innej funkcjonalno�ci.


3) Definiowanie nowych typ�w
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Istniej� dwa rodzaje typ�w: typy w�a�ciwe, oraz podtypy. O ile typy
w�a�ciwe nie daj� si� przepisywa� mi�dzy sob� (np. nie mo�na przypisa�
bezpo�rednio warto�ci zmiennej typu Typ1 do zmiennej typu Typ2, nawet
je�li za oboma typami kryje si� tylko typ unsigned), o tyle podtypy
mog� by� przpisywane mi�dzy sob�. A wi�c Podtyp1 i Podtyp2, o ile s�
podtypami tego samego typu, daj� si� mi�dzy sob� przepisywa�.

Aby zdefiniowa� typ, u�ywa si� nast�puj�cej dyrektywy:

[sub]type nazwa_typu is typ_bazowy [ range aa .. bb ];

type nazwa_typu is array aa .. bb of typ_bazowy;

type nazwa_typu is bytechunk aa .. bb;

type nazwa_typu is structure {
  obiekt : [modyfikatory] typ;
  ...
}

U�ycie modyfikatora "range" mo�liwe jest dla typ�w prostych (i tylko dla 
nich). Dzi�ki jego wykorzystaniu mo�na ograniczy� zakres warto�ci, kt�re
mog� by� przyjmowane przez dan� zmienn�.

Modyfikator array powoduje utworzenie tablicy obiekt�w typu typ_bazowy,
kt�ry mo�e by� albo typem predefiniowanym, albo typem zdefiniowanym przez
u�ytkownika. Nie jest mo�liwe tworzenie podtyp�w tablicowych, poniewa�
nie mia�oby to wi�kszego sensu. W przypadku tablicy, jest mo�liwa 
deklaracja umo�liwiaj�ca podanie jej rozmiaru w chwili definiowania
zmiennej:

type nazwa_typu is variable array of typ_bazowy;

Wtedy definicja zmiennej musi wygl�da� np. nast�puj�co:

zmienna : nazwa_typu(1,10);

Mo�liwa jest tak�e deklaracja typu z pomini�ciem jednego lub drugiego
parametru:

type nazwa_typu is variable array start 1 of typ_bazowy;
type nazwa_typu is variable array end 10 of typ_bazowy;

Albo z pominieciem obu parametrow:

type nazwa_typu is variable array of typ_bazowy;

Wtedy deklaracja zmiennej musi sie odbyc po podaniu jednego lub dwoch
parametrow (nie dotyczy typu wskaznikowego).

Obok 'array', istnieje takze analogiczny obiekt - bytechunk. Uzywa sie
go w deklaracjach zupelnie tak, jak w przpadku tablic, z pominieciem
specyfikacji typu bazowego:

type nazwa is bytechunk a..b;
type nazwa is variable bytechunk [ start nn | end nn ];

Tworzony zostaje w ten sposob spojna tablica bajtow, idealna do przechowywania
np. ciagow znakow. UWAGA: typy oparte na bytechunk SA tozsame. Mozliwe sa
miedzy nimi przepisania, pod warunkiem, zgodnosci rozmiarow, jak rowniez 
przypisania dowolnego obiektu bytechunk do dowolnego wskaznika do dowolnego
innego typu opisanego jako bytechunk.

Modyfikator structure { ... } pozwala zdefiniowa� struktur�, czyli blok
sk�adaj�cy si� z obiekt�w sk�adowych, wymienionych w segmencie { ... }.
W sk�ad struktury mog� wchodzi� dowolne obiekty o dowolnym typie lub
wska�niki do tych obiekt�w, o ile zosta�y zdefiniowane wcze�niej.

W celu jednoznaczno�ci deklaracji nie jest mo�liwe definiowanie typ�w
bazuj�c na anonimowych typach bazowych. Nie jest wi�c poprawn� konstrukcja:

type nazwa_typu is array aa .. bb of structure {
  ...
};

Poniewa� nie jest okre�lona nazwa struktury. Deklaracj� t� nale�y roz�o�y�
na dwie oddzielne, pierw okre�laj�c nazw� struktury.

Ka�da zmienna posiada pewne w�a�ciwo�ci, dost�pne w trybie read-only. S� to:

Zmienna'First		- dolna granica range lub dolna warto�� tablicy
Zmienna'Last            - jw, g�rna
Zmienna'Count           - 'Last - 'First
Zmienna'Byte_Length     - rozmiar obiektu w bajtach
Zmienna'Dword_Length    - rozmiar obiektu w dwords
Zmienna'Address		- adres zmiennej (dost�pny tylko je�li zmienna
                          zosta�a zdefiniowana z parametrem addressable lub
                          jest wska�nikiem)

Warto�ci inicjalizuj�ce dla bytechunk'a mog� by� list� warto�ci liczbowych
tak jak w przypadku tablicy ({1,2,3}) lub tekstem uj�tym w cudzys�owy
("tekst").

W obecnej chwili struktury nie beda wspieraly inicjalizatorow pol oraz
korzystania z typow, ktore wymagaja inicjalizacji - zmieni sie to 
prawdopodobnie w nastepnej wersji AHLL.


4) Wska�niki
~~~~~~~~~~~~

Wska�niki wyst�puj� w formie niejawnej, podobnie jak w Adzie. Jest wi�c
mo�liwe stworzenie zmiennej z atrybutem "pointer to", ale od chwili
przypisania jej warto�ci do jej wymazania, obiekt ten podlega dok�adnie
takim samym prawom, jak zwyk�y obiekt danego typu. Mo�liwe s� nast�puj�ce
operacje:

Create(zmienna);	- stworzenie nowego obiektu i przypisanie go "pod"
			  wska�nik "zmienna"; operacja powiedzie si� tylko
		          je�li zmienna jest obiektem wska�nikowym i nie
                          jest do niej w danej chwili zbindowana warto��.

Unbind(zmienna);        - od��czenie (odbindowanie) wska�nika od zmiennej;
                          normalnie, gdy zostanie opuszczona funkcja, w
                          kt�rej zaalokowana zosta�a warto�� z wykorzystaniem
                          Create(); dopiero po tej operacji mo�liwe jest
                          ponowne wywo�anie Create() na danej zmiennej.

Bind(zmienna,nowa);	- przy��czenie (zbindowanie) wska�nika do istniej�cej
			  zmiennej 'nowa'; mo�liwe tylko je�li zmienna 'nowa'
			  by�a zdefiniowana z atrybutem addressable.
                          Jest to rownowazne z konstrukcja zmienna := nowa;

Destroy(zmienna);	- zniszczenie obiektu (tylko je�li by� stworzony
                          funkcj� Create).


Dla wygody, zmienne wska�nikowe udost�pniaj� dwa dodatkowe atrybuty:

Zmienna'Binded		- czy wska�nik jest zbindowany?
Zmienna'Used		- czy jest przypisana warto��?


4) Range-checking i wyj�tki
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Range checking przeprowadzany jest w nast�puj�cych sytuacjach:

- jawna konwersja typu funkcji
- niejawna konwersja (przypisanie)
- dost�p do tablicy
- dynamiczne tworzenie obiektu w bloku local { ... }

Kompilator generuje kod zg�aszaj�cy wyj�tki w nast�puj�cych sytuacjach:

- B��dy range checkingu
- Create() / Bind() na zbindowanym wska�niku
- Destroy() na niezbindowanym wska�niku
- Unbind() na niezbindowanym wska�niku
- ...plus wyj�tki systemowe

Przy innych b��dach (typu Create() na obiekcie nie b�d�cym wska�nikiem,
Bind() do obiektu kt�ry nie jest addressable) powinny by� zg�aszane b��dy
kompilacji, co jest proste.


5) Procedury
~~~~~~~~~~~~

W AHLL nie ma funkcji, tylko procedury. Procedura wygl�da nast�puj�co:

[ addressable ]
procedure nazwa_procedury ( [writable] par1 : typ1, par2 : typ2 ... ) {
  [ local {
    ...
  }
  [ protected {
    ...
  } ]
  kod procedury
  ...
  [ exception {
    ...
  } ]
}

Pierw deklarowane s� parametry, wraz z opisuj�cymi je nazwami typ�w. Potem,
nast�puje blok local { ... }, gdzie mog� znajdowa� si� deklaracje lokalnych
zmiennych. Potem znajduje si� normalny kod - sk�adaj�cy si� z przypisa�,
konstrukcji warunkowych, p�tli oraz wywo�a� innych procedur. Ko�cowy blok
exception { ... }, om�wiony w sekcji 7, odpowiada za obs�ug� wyj�tk�w :>
Blok protected { ... } jest kodem, w obr�bie kt�rego maj� by� obs�ugiwane
wyj�tki (mo�e wyst�powa� dowolnie cz�sto).

Modyfikator addressable przed nazw� procedury pozwala na pobranie jej
adresu (typu proc_addres) przez udost�pnienie w�asno�ci 'IP_Address


6) Syscalle i standardowe funkcje
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Jako interfejs do kernela, istnieje specjalna dyrektywa syscall o
nast�puj�cej postaci:

  syscall(numer, u0 := nn, u1 := nn, ..., nn := u0 ...);

Numer syscalla jest oczywi�cie systemowym numerem wywo�ania. Nast�pnie
wyst�puje blok przypisa� do specjalnych zmiennych o nazwach uX, sX lub
fX, czyli rejestr�w VCPU, oraz blok przypisa� Z rejestr�w ( nn = uX, sX, fX).
Pierwszy blok oznacza parametry dla wywo�ania syscalla, drugi - warto�ci
powrotne. A wi�c na przyk�ad:

// To powinno znale�� si� w pliku nag��wkowym do obs�ugi �a�cuch�w tekstowych.

type UnBString is variable bytechunk;

type BString is structure {
  text : pointer to UnBString;
  len  : unsigned;
}

type Time_Type is unsigned; // Zeby uniknac przypadkowych przepisan :)

procedure Convert_Time_To_String(Time : Time_Type, writable out : BString) {

  syscall(LOCAL_TIMETOSTR, u[0] := Convert(Time), u[1] := out.text'Address,
                           u[2] := out.len, out.len := s[0]);

}


7) Obs�uga wyj�tk�w
~~~~~~~~~~~~~~~~~~~

S�u�y do nich blok exception { }, opcjonalny na ko�cu ka�dej procedury.
Zostanie on wykonany w przypadku zg�oszenia wyj�tku. Jego budowa jest
podobna jak dla switch:

exception {
  wartosc: kod [...]
  wartosc: kod [...]
  [...]
  [ default: kod... ]
}

Zg�aszanie wyj�tk�w mo�e si� odbywa� z wykorzystaniem predefinowanej
dyrektywy raise(numer_wyj�tku). Nieobs�u�ony wyj�tek prowadzi do zako�czenia
pracy programu.

Zako�czenie pracy programu jest mo�liwe tak�e za pomoc� dyrektywy halt().

Mo�e to nienajlepsze miejsce, ale zawsze. Inne predefiniowane dyrektywy
dotycz�ce wykonywania programu:

  cycle_wait(nn) - wprowadzenie zadania w stan CWAIT
  time_wait(nn)  - wprowadzenie zadania w stan TWAIT


8) Arytmetyka i odwo�ania do tablic/struktur
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Odwo�ania do element�w tablic: tablica[numer_elementu]
Odwo�ania do element�w struktury: struktura.nazwa_elementu
Odwo�ania mog� by� kombinowane - np. struktura.nazwa_tablicy[numer_elementu].

Przypisania:

  zmienna := wartosc;

Operatory arytmetyczne (w miar� mo�liwo�ci powinny by� liczone w trakcie
kompilacji): +, -, *, / - standardowe operacje.

Operatory binarne: AND, OR, XOR, NOT.

Operatory logiczne por�wna�: <>, =, <, >, >=, <=.

Kolejno�� oblicze�: ().

Przyk�ad:

  zmienna := ( 2 > 1 ); // Przypisana b�dzie warto�c 1 ;)

Nie ma oddzielnego typu logicznego.


9) Bloki warunkowe, kontrola wykonywania
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Wywo�anie procedury: nazwa_procedury(parametr,parametr...);
Deklaracja labela: :nazwa_labela
Skok do labela (musi byc w tej samej funkcji): goto nazwa_labela

Zako�czenie programu: terminate();

Wykonywanie warunkowe:

  if (wyrazenie_niezerowe) {
    ...
  } [ elif (wyrazenie niezerowe | else { ... } 

P�tle:

  do while (wyrazenie_niezerowe) {
    ...
  }

  do {
    ...
  } while (wyrazenie_niezerowe);

Kontrola p�tli: continue, break - jak w C

Wyj�cie z procedury: return

W bloku exception {} dopuszczalne s�:

  return   - powrot z procedury
  ignore   - kontynuowanie w punkcie, gdzie wyst�pi� wyj�tek

  terminate(), rzecz jasna
  

Bloki switch:

  switch (wartosc) {
    mozliwosc: ...kod...
    mozliwosc: ...kod...
    default: ...kod...
  }

Default case musi istnie�, mo�e by� pusty.

10) Przyk�adowy program
~~~~~~~~~~~~~~~~~~~~~~~

-- hello.ahl --
#include "stdinc.ahh"
#include "display.ahh"

procedure Main() {
  Console_PutString( { "Hello world.\n", Self[1]'Byte_Length } );
}
-- EOF --


--------------------------------------
13. Specyfikacja modu��w standardowych
--------------------------------------


  Modu� display.c
  ---------------

  Status: zamro�ony

  Przeznaczenie: wy�wietlanie podstawowych danych na konsoli z poziomu
  procesu u�ytkownika; debugging etc. UWAGA: modu� nie powinien by�
  u�ywany do interakcji z u�ytkownikiem. Powstanie do tego inne rozwi�zanie,
  sugerowanym obecnie sposobem jest modu� obs�ugi sieci.

  Syscall: IO_PUTSTRING

            Parametry: u0 - adres ci�gu znak�w
                       u1 - ilo�� znak�w

            Efekt: wy�wietla ci�g znak�w

            Wyj�tki: BAD_PROTFAULT - pr�ba wy�wietlenia b��dnego fragmentu
                     pami�ci

            HAC: operacja=display/output/text obiekt=none

  Syscall: IO_PUTINT

            Parametry: u0 - warto�� do wy�wietlenia

            Efekt: wy�wietla warto�c liczbow�

            Wyj�tki: -

            HAC: operacja=display/output/integer obiekt=none

  Syscall: IO_PUTCHAR

            Parametry: u0 (najm�odsze 8 bit�w) - znak do wy�wietlenia

            Efekt: wy�wietla znak ascii

            Wyj�tki: -

            HAC: operacja=display/output/character obiekt=none


  Modu� access.c
  --------------

  Status: zamro�ony

  Przeznaczenie: zarz�dzanie przywilejami; zmiana aktywnej domeny (grupy)
  i identyfikatora podgrupy. Wsparcie systemu HAC.

  Kontrola dost�pu: brak.

  Syscall: ACCESS_SETDOMAIN

            Parametry: u0 - numer grupy

            Efekt: zmienia aktywn� grup�

            Warunek: grupa nale�y do zbioru !domains okre�lonego w czasie
                     kompilacji

            Wyj�tki: NOPERM - je�li grupa nie znajduje si� w w/w
                               zbiorze.

            HAC: brak wsparcia

  Syscall: ACCESS_SETUID

            Parametry: u0 - numer identyfikatora podgrupy

            Efekt: zmienia aktywn� podgrup�

            Warunek: brak

            Wyj�tki: brak

            HAC: brak wsparcia


  Modu� fs.c
  ----------

  Status: wdra�anie

  Przeznaczenie: dost�p do SVFS.

  Kontrola dost�pu: HAC + istnienie obiekt�w w hierarchi SVFS.
  Wyj�tki: standardowe + FSERROR - b��d dost�pu do zasob�w SVFS.

  Syscall: FS_OPEN_FILE

            Parametry: u0 - adres nazwy pliku, u1 - d�ugo�c nazwy pliku
                       u2 - flagi: FS_FLAG_READ, FS_FLAG_WRITE,
                                   FS_FLAG_APPEND, FS_FLAG_NONBLOCK
 
            Efekt: otwiera podany plik w odpowiednim trybie
                   s0 - VFD (virtual file descriptor); -1 = plik zalockowany

            Uwagi: je�li nie jest podana flaga NONBLOCK, a zg�oszono
                   pr�b� otwarcia pliku do zapisu i jednocze�nie inny
		   proces zapisuje dane do pliku, proces wchodzi w stan
                   IOWAIT do czasu uzyskania dost�pu; NONBLOCK powoduje
		   natychmiastowe zwr�cenie warto�ci -1.

            HAC: fs/fops/open/file/{read|write|append}

  Syscall: FS_CREATE_FILE

            Parametry: u0 - adres nazwy pliku, u1 - d�ugo�c nazwy pliku
                       u2 - flagi: FS_FLAG_WRITE, FS_FLAG_APPEND
 
            Efekt: tworzy podany plik w odpowiednim trybie
                   zwraca s0 - VFD (virtual file descriptor)

            HAC: fs/fops/create/file/{write|append}


  Syscall: FS_CLOSE_FILE

            Parametry: u0 - numer VFD
 
            Efekt: zamkni�cie VFD, koniec pracy z plikiem

            HAC: brak


  Syscall: FS_WRITE_FILE

            Parametry: u0 - numer VFD, u1 - wska�nik, u2 - d�ugo�� (bajty!)
 
            Efekt: zapisanie danych do pliku, je�li pozwalaj� na to
                   uprawnienia dost�pu do VFD i pami�ci

            HAC: brak


  Syscall: FS_READ_FILE

            Parametry: u0 - numer VFD, u1 - wska�nik, u2 - d�ugo�� (bajty!)
 
            Efekt: odczyt danych z pliku do pami�ci, je�li pozwalaj� na to
                   uprawnienia dost�pu do VFD i pami�ci

            HAC: brak


  Syscall: FS_SEEK_FILE

            Parametry: u0 - numer VFD, u1 - pozycja, u2 - typ
 
            Efekt: sk�adania analogiczna do lseek() w libc. Na plikach
		   dost�pnych w trybie append() tylko u1=0, u2=1 (current)
                   jest akceptowane (zwraca obecn� pozycj�).

                   s0 - pozycja.

            HAC: brak


  Syscall: FS_MAKE_DIR

            Parametry: u0 - nazwa, u1 - dlugosc nazwy
 
            Efekt: utworzenie katalogu ;)

            HAC: fs/fops/create/directory


  Syscall: FS_DELETE

            Parametry: u0 - nazwa, u1 - dlugosc nazwy
 
            Efekt: usuniecie pliku lub katalogu

            HAC: fs/fops/delete/{directory|file}


  Syscall: FS_RENAME

            Parametry: u0 - nazwa, u1 - dlugosc nazwy
                       u2 - nowa nazwa, u3 - dlugosc nowej nazwy
 
            Efekt: zmiana nazwy pliku lub katalogu

            HAC: fs/fops/delete/{directory|file} dla starej nazwy
                 fs/fops/create/{directory|file} dla nowej nazwy


  Syscall: FS_PWD

            Parametry: u0 - bufor, u1 - rozmiar bufora
 
            Efekt: wpisanie do bufora aktualnego katalogu, wpisanie
                   do s0 faktycznej d�ugo�ci nazwy

            HAC: brak


  Syscall: FS_CWD

            Parametry: u0 - bufor, u1 - rozmiar bufora
 
            Efekt: zmiana aktualnego katalogu pracy (nie jest weryfikowane,
                   czy katalog istnieje!)

            HAC: brak


  Syscall: FS_STAT

            Parametry: u0 - nazwa zasobu, u1 - dlugosc nazwy
 
            Efekt: u0 - czas ostatniej modyfikacji
                   u1 - 0 = brak dostepu do zasobu
                        1 = zasob to plik
                        2 = zasob to katalog

            HAC: fs/fops/stat

   [ pozosta�o czytanie katalogu - do zrobienia ]


  Modu� locallib.c
  ----------------

  Status: wdra�anie

  Przeznaczenie: dost�p do zasob�w systemowych.

  Kontrola dost�pu: HAC
  Wyj�tki: standardowe

  Syscall: LOCAL_GETTIME

            Efekt: u0 - sekundy, u1 - mikrosekundy

            HAC: local/sys/real/time/get

  Syscall: LOCAL_TIMETOSTR

            Parametry: u0 - zwr�cone przez GETTIME, u1 - adres bufora,
                       u2 - rozmiar bufora

            Efekt: wpisuje string do bufora, w s0 zwraca ilo��
                   znak�w.

            HAC: brak

  Syscall: LOCAL_GETHOSTNAME

            Parametry: u0 - adres bufora, u1 - rozmiar bufora

            Efekt: wpisuje lokaln� nazw� komputera do bufora, s0 - ilo��
                   znak�w.

            HAC: local/sys/real/hostname/get

  Syscall: SYSCALL_GETRANDOM

	    Efekt: u0 - losowy dword; funkcja pobiera dword z lokalnego
                        �r�d�a entropii (/dev/urandom)

            HAC: local/sys/random/get

   Syscall: SYSCALL_LOCAL_VS_STAT

            Efekt: u0 - ilo�� aktywnych VCPUs; u1 - ilo�� cykli idle od startu,
                   u2 - ilo�� cykli pracy od startu, u3 - ilo�� syscalli,
                   u4 - ilo�� z�ych syscalli, u5 - fatal errors...

            HAC: local/sys/virtual/stat 

   Syscall: SYSCALL_LOCAL_RS_STAT

            Efekt: u0 - uptime w sekundach, u1 - load average (1 min),
                   u2 - ilo�� RAM w kB, u3 - wolny RAM w kB, u4 - ilo��
                   swapa w kB, u5 - wolny swap w kB, u6 - ilo�� proces�w RS.

            HAC: local/sys/real/stat





UWAGA: Ka�dy modu� korzystaj�cy z HAC, opr�cz standardowych wyj�tk�w, mo�e
tak�e zwr�ci�: ACL_PROBLEM, NOPERM.

Szkic pozosta�ych modu��w:

Obecnie tworzone modu�y:

advmem.c        - zaawansowane operacje na pami�ci (w celu przyspieszenia
                  z�o�onych zada� wyszukiwania, por�wnywania, przepisywania)

                  Kontrola dost�pu: brak

locallib.c	- przydatne funkcje systemowe - odczyt zegara czasu
                  rzeczywistego etc.

		  Kontrola dost�pu: HAC

ipc.c           - komunikacja mi�dzyprocesowa - nawi�zywanie sesji, wysy�anie
                  i odbieranie danych, wsp�dzielenie pami�ci.

                  Kontrola dost�pu: HAC

network.c       - dost�p do us�ug sieciowych (tcp, unix sockets)

                  Kontrola dost�pu: HAC

remote_ipc.c    - remote IPC. Modu� zast�puj�cy IPC z funkcjami serwer-klient
                  i mo�liwo�ci� zestawiania po��cze�

                  Kontrola dost�pu: HAC


---------------------
14. Tworzenie modu��w
---------------------

W obecnym rozwi�zaniu s� to dynamicznie linkowane programy w C lub Adzie.
Wymagania s� nast�puj�ce:

- istnienie syscall_load(int* x); funkcja ta wywo�ywana jest w chwili
  wczytania modu�u, ma on obowi�zek wype�ni� tablic� warto�ci x numerami
  syscalli, kt�re ma zamiar obs�ugiwa�; warto�ci tych syscalli znajduj� si�
  w pliku syscall.h (oczywi�cie dla nowych funkcji nale�y doda� nowe i
  umie�ci� je w syscall.h). Lista nie mo�e przekracza� MAX_SERVE z pliku
  config.h i musi by� zako�czona warto�ci� ujemn�.

- istnienie syscall_handler(int c,int sysnum) - ta funkcja b�dzie wywo�ana
  je�li VCPU o numerze 'c' wywo�a syscalla o numerze znajduj�cym si� na
  li�cie zarejestrowanej dla tego modu�u (konkretny numer podawany jest w
  sysnum). Warto�� 'c' pozwala na odwo�ywanie si� do struktury vcpu_struct
  zdeklarowanej w task.h (odsy�am po szczeg�y).

- opcjonalnie, istnienie syscall_unload, wykonywanego przy ko�czeniu pracy
  syscalla,

- opcjonalnie, istnienie syscall_task_cleanup, kt�ry zostanie wykonany
  zawsze gdy jakie� zadanie ko�czy prac� (usuni�cie otwartych deskryptor�w
  itp),

D�ugo mo�na omawia� budow� modu��w, wi�c wklej� tu fragment przyk�adowego,
obs�uguj�cego prymitywne wyj�cie na konsole:

--
void syscall_load(int* x) {
  *x=SYSCALL_IO_PUTSTRING;
  *(++x)=SYSCALL_ENDLIST;
  printk("<< Welcome to I/O module >>\n");
}

void syscall_handler(int c,int num) {

  int cnt;
  int from;
  char* start;

  if (num==1) {
    from=cpu[c].uregs[0];
    cnt=cpu[c].uregs[1];
    start=verify_access(c,from,(cnt+3)/4,MEM_FLAG_READ);
    if (!start) {
      non_fatal(ERROR_PROTFAULT,"Can't print non-accessible memory",c);
      return;
    }
    write(2,start,cnt);
  }

}
--

Wywo�anie funkcji non_fatal s�u�y do zg�aszania wyj�tk�w.

Wymiana bibliotek: za�adowanie nowej do dowolnego wolnego slotu, nast�pnie
od�adowanie starej z jej pierwotnego slotu. Nie nast�pi przerwa w obs�udze
syscalli.

Aha, syscalle NIE mog� blokowa� pracy systemu - dok�adnie jak w np. Linuxie.
Dlatego je�li konieczne jest oczekiwanie na jak�� operacj� (np. recv()),
zaleca si� ustawienie stanu procesu (cpu[nn].state) dodaj�c flag�
VCPU_STATE_IOWAIT i rownoczesne ustawienie cpu[nn].iohandler tak, by
wskazywa� na funkcj� akceptuj�c� pojedynczy parametr (numer VCPU):
int handler(int cpu_num);

Dodatkowo dost�pne jest pole cpu[nn].iowait_id, okre�laj�ce identyfikator
zasobu, na kt�ry proces oczekuje.

Od tej chwili, proces nie b�dzie pracowa� (sytuacja analogiczna do
STATE_SLEEP). Zamiast tego, w ka�dym cyklu obs�ugi zada�, b�dzie wywo�ywana
funkcja iohandler(numer_cpu). Funkcja powinna sprawdzi� numer zasobu,
na kt�ry zadanie oczekuje. Je�li dost�p jest niemo�liwy, powinna zwr�ci�
0. Je�li sta� si� mo�liwy, funkcja powinna odpowiednio obs�u�y� wyniki,
a nast�pnie zwr�ci� warto�� niezerow� (np. 1) aby automatycznie opu�ci�
stan IOWAIT.

Dany modu� powinien sam zadba� o przechowywanie informacji na temat tego,
gdzie dla danego zadania zapisa� informacje powrotne, etc.

Do wej�cia w stan IOWAIT najbezpieczniej u�y� makra:

ENTER_IOWAIT(numer_cpu,numer_zasobu,iohandler)

Nale�y pami�ta�, by nie przekazywa� ani nie pobiera� od procesu "go�ych"
obiekt�w rzeczywistego systemu - np. numer�w deskryptor�w plik�w, ani
nie powierza� systemowi kontroli uprawnie� (np. pr�bowa� pisa� do
deskryptora, a potem sprawdza� czy wysz�o). Argante zapewnia pe�n� kontrol�
po w�asnej stronie, w zunifikowany spos�b, za� wszystkie "rzeczywiste"
obiekty przechowuje w oddzielnych dla ka�dego procesora tablicach,
podaj�c procesowi co najwyj�ej identyfikator w obr�bie tych tablic. Najlepszym
przyk�adem poprawnej konstrukcji modu��w jest modu� fs.

Oto kr�tki opis filozofii obs�ugi string�w na niskim poziomie (co nie
interesuje zapewne programisty AHLL, ale jest istotne przy tworzeniu modu��w),
kt�ry napisa�em dla Artura:

[...]

Och, kiedy wlasnie gethostbyname jest wcale niezlym przykladem. Generalnie
robimy to tak:

- user przekazuje nam adres bufora i jego rozmiar (w rejestrach kolejno
  u0 i u1, powiedzmy).

- sprawdzamy, czy user ma prawo wykonac dana operacje - w twoim przypadku
  wystarczy makro: VALIDATE(c,"none","local/sys/real/uname/get");
  Makro samo zrobi 'return' jesli user nie ma praw do danego obiektu.

- Musisz sprawdzic, czy przekazany przez usera adres na calej swej
  rozciaglosci daje sie zapisywac - jesli nie, oczywiscie nie mozemy
  obsluzyc jego syscalla i zglaszamy wyjatek:

  if (!(sth=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,
        MEM_FLAG_WRITE))) {
    non_fatal(ERROR_PROTFAULT,"gethostname: Attempt to access protected"
                              " memory",c);
    failure=1;
    return;
  }
  verify_access akceptuje parametry kolejno: numer VCPU, adres,
  rozmiar (ale uwaga - w dwords - tak wiec podany rozmiar w bajtach
  musimy przeliczyc; jako ze operator '/' w C na intach to po prostu
  idiv, olewajacy reszte z dzielenia, upewniamy sie, zeby "zlapac"
  przypadek typu: user mowi, ze mozna zapiac 1 bajt, 1/4 wg idiv = 0,
  wiec sprawdzmy 0 bajtow ;), oraz typ dostepu (READ albo WRITE).

  Funkcja zwraca albo wskaznik (juz w rzeczywistym systemie, normalny
  void*) albo NULL - to znaczy, ze nie ma praw dostepu do bloku i
  powinnismy zglosic wyjatek, ustawic failure (taka konwencja, dla
  wlasnej wygody, jakies to mialo uzasadnienie ;) i przerwac dalsza
  prace.

- Oka, sukces, zalozmy ze mamy juz wskaznik - pobieramy wiec co trzeba,
  wpisujemy max. uregs[1] bajtow pod adres zwrocony przez verify_access,
  a nastepnie zwracamy (powiedzmy w s0) ilosc znakow, ktora pobralismy.

  UWAGA: nie kopiujemy ani nie liczymy NULL-terminatora, ktory w
  Argante jest normalnym znakiem. Nie robimy wiec np:

  strncpy(sth,jakis_bufor,cpu[c].uregs[1]); tylko:

  memcpy(sth,jakis_bufor,strlen(jakis_bufor)) i zwracamy
  strlen(jakis_bufor) w s0. Aha, wczesniej musimy sprawdzic,
  czy strlen(jakis_bufor)>cpu[c].uregs[1] (czyli czy nie bedziemy
  chcieli zapisac wiecej, niz trzeba) i ewentualnie musimy
  zglosic wyjatek.

To tyle o stringach w user-space z punktu widzenia kernel-space. Nie, nie
sa przewidywane stringi powiazane na sztywno z bajtem/wordem/dwordem
oznaczajacym ich dlugosc - to bedzie kwestia gustu i implementacji w HLL,
natomiast kernelowi informacje przekazywane sa luzem :P

Wieksze klopoty ze stringami ma tylko biedny z33d, ktory musi wprowadzac
dodatkowa wartosc przy niektorych operacjach ;) Tzn. albo zwracac offset w
bajtach, albo adres argante + 0..3 offsetu ;> Ale to tez nie jest wielki
problem.

[ z33d pisal modul advmem, obslugujacy m.in. sklejanie / przeszukiwanie
  tesktow itp ]


-------------------------------
15. Format plik�w wykonywalnych
-------------------------------

Format nag��wka pliku wykonywalnego jest nast�puj�cy:

  unsigned int magic1;

    Sta�a sygnatura pliku, warto�� 0xdefaced

  char domains[MAX_EXEC_DOMAINS];

    Lista domen, do kt�rych przynale�y program, zako�czona warto�ci�
    zerow�.

  unsigned int flags;

    Pocz�tkowe flagi procesu. W obecnej chwili �adne flagi nie s�
    supportowane.

  unsigned int priority;

    Priorytet okre�la, jak d�ugi timeslice jest przyznawany procesowi w
    ka�dym cyklu obs�ugi. Priority wynosz�ce 1 powoduje, i� proces za ka�dym
    razem mo�e wykona� 1 instrukcj�.

  unsigned int ipc_reg;

    To pocz�tkowy identyfikator IPC. Je�li jest warto�ci� dodatni�, zostanie
    przepisany do VCPU.

  unsigned int init_IP;

    Pocz�tkowy instruction pointer, zwykle wystarczy 0.

  int current_domain;
  int domain_uid;

    Aktualna domena wykonywania i UID. Uwzgl�dniane tylko gdy s�
    warto�ciami dodatnimi.

  unsigned int bytesize;

    Rozmiar obrazu kodu.

  unsigned int memflags;

    Flagi pami�ci (READ|WRITE, etc)...

  unsigned int datasize;

    Rozmiar obrazu danych.

  char signature[64];

    Opcjonalna sygnatura autora / kr�tki opis programu.

  unsigned int magic2;

    Sta�a sygnatura 0xdeadbeef

Nast�pnie nast�puje blok obrazu kodu (rozmiar - 12*bytesize) oraz blok
danych (opcjonalny, rozmiar 4*datasize). Oba bloki s� mapowane od adresu 0
odpowiednio w przestrzeni kodu i danych.

Do modyfikacji tre�ci nag��wk�w ju� istniej�cych program�w s�u�y program
Bulby - dost�pny w tools/binedit.c

---------------------- 
16. Wbudowany debugger
----------------------

Autor: z33d

Bez kompromisow ? Ok, nie bede uzywal polskich liter.

Zaimplementowalem skromny, lecz mysle, ze w pelni funkcjonalny system
debugowania interaktywnego. Cos na ksztalt gdb. Aby zainicjowac debugowanie
procesu, nalezy zaladowac go poprzez komende 'd'. Wowczas zostanie
ustawiona flaga VCPU_FLAG_DEBUG, ktora niezbedna jest do korzystania z
funkcji debuggera. 

Kawalek helpa: ('?')
  dfn         - load and run binary in debug mode
  rnn         - show nn vCPU registers
  xnn addr c  - show c bytes of memory on nn vCPU
  nnn         - step exactly one instruction of nn vCPU
  cnn         - continue process on nn vCPU
  snn         - continue process on nn vCPU to next syscall
  fnn         - continue process on nn vCPU to next ret
  lnn         - list breakpoints on nn vCPU
  bnn zz      - add breakpoint on nn vCPU at zz IP
  unn zz      - delete zz breakpoint on nn vCPU
  inn IP c    - disassemble c instructions at IP on VCPU nn
  tnn         - show stack trace on nn vCPU

Komendy sa chyba jasne, dodatkowo kazde wywolanie wyjatku (nawet przechwycone)
powoduje zatrzymanie wykonywania procesu. (VCPU_STATE_STOPPED)

Opr�cz debuggera, z33d napisa� tak�e deassembler, znajduj�cy si� w katalogu
tools/. Nie jest jeszcze doko�czony, tym niemniej swoj� rol� spe�nia dobrze.


-------
17. FAQ
-------

Poni�ej znajduje si� niewielki zbi�r odpowiedzi na najcz�ciej zadawane 
pytania. Wi�kszo�� tych odpowiedzi zawiera informacje znajduj�ce si� ju�
wy�ej, ale czasem �atwo co� przeoczy� - w ka�dym razie cz�sto jeste�my
pytani w nast�puj�cy spos�b:

1) Po co to wszystko?

Dla przyjemno�ci. Piszemy Argante nie dlatego, i� chcemy stworzy� drugiego
Linuxa - raczej chcieli�my sprawdzi�, czy jest mo�liwe zaprojektowanie
systemu, kt�ry ��czy�by bezpiecze�stwo z funkcjonalno�ci�, wydajno�ci�,
uniwersalno�ci� - a r�wnocze�nie zrywa�by z wi�kszo�ci� konwencji spotykanych
w innych systemach. Po drugie, chcieli�my sprawdzi�, czy potrafimy to
zrobi� w�asnymi si�ami :)

Zupe�nie inn� kwesti� jest to, i� niekt�re rozwi�zania Argante mog� sta�
si� potencjalnie ciekawym k�skiem - jak na przyk�ad mo�liwo�ci zarz�dzania
(plug and play) i tworzenia warstwy komunikacyjnej wewn�trz clustr�w,
niezale�nie od stopnia rozproszenia system�w, w spos�b transparentny dla
programisty.

Mimo to, nie chcemy, by ten system sta� si� produktem, dlatego zdecydowali�my
si� na rozpowszechnianie go od chwili sko�czenia (co, mam nadziej�, nast�pi
w przeci�gu kilku tygodni) na licencji GPL. Produktem mo�e by� oprogramwanie,
support, konkretne rozwi�zania.

2) Gdzie widzicie zastosowania Argante?

Wszelkie rozproszone serwery, na kt�rych bezpiecze�stwo i efektywno��
jest zagadnieniem krytycznym, wspomniane wy�ej clustry, oraz wiele innych
zastosowa�. Nie, nie oczekujemy, i� Argante stanie si� produktem typu
desk-end - nie chcemy konkurowa� ani z Microsoftem, ani nie zamierzamy
powtarza� sukcesu Linuxa.

3) Czy Argante b�dzie oddzielnym systemem?

Wspomina�em o tym, ale wszystko zale�y od tego, jak b�dzie si� rozwija�.
System osadzony posiada swoje zalety - m.in. mo�liwo�� dok�adnej
integracji (w ramach wspomnianych rozwi�za� hybrydowych) z rzeczywistym
systemem, oraz brak konieczno�ci przenoszenia ca�o�ci oprogramowania za
jednym zamachem na nowy OS.

4) Jak z przeno�no�ci� aplikacji z Unixa?

Nie b�dzie czego� takiego, bo Argante operuje na zupe�nie innych zasadach.
Mo�na m�wi� o przeno�no�ci typu DOS <-> Unix - i tu i tu zadzia�a program
"Hello world", ale �adne powa�niejsze zastosowania, ze wzgl�du na ogromne
r�nice, nie b�d� przeno�ne. Dlatego nie starali�my si� nawet kopiowa�
j�zyka typu C, cho� oczywi�cie nic nie stoi na przeszkodzie, by kto�
napisa� C dla Argante - aczkowiek nie jest to bezpieczny j�zyk...

5) Czemu Argante ma w�asny j�zyk?

W�a�ciwie Argante mog�aby operowa� na podzbiorze Ady - z drugiej strony
wiele konwencji C dotycz�cych sterowania kodem wyda�a nam si� nieszkodliwym
zapo�yczeniem :) Dlatego AHLL jest po��czeniem dobrych cech obu j�zyk�w
w ramach niewielkiego i bardzo �atwego do opanowania (podobnie jak C, cho�
bez tak chorej arytmetyki na wska�nikach itp) podzbioru.

6) Czy moge zmienia� parametry systemu?

Maksymalna ilo�� VCPUs, maksymalny rozmiar stosu, a tak�e wi�kszo��
innych parametr�w charakteryzuj�cych �rodowisko, mo�e by� modyfikowana
w pliku config.h; nale�y pami�ta�, �e zmiana pewnych witalnych
parametr�w (np. ilo�ci rejestr�w) mo�e prowadzi� do niekompatybilno�ci
/ nieprzeno�no�ci program�w lub nieprawid�owego ich dzia�ania.

7) Jak Argante wykorzystuje moc procesora?

Gdy wszystkie procesy s� "martwe", oczekuj� na okre�lony moment lub
znajduj� si� w stanie IOWAIT, zegar VS zwalnia znacznie, oddaj�c wi�kszo��
mocy procesora rzeczywistemu systemowi. W przypadku, gdy cho� jeden lub
wi�cej proces�w znajduje si� w stanie WORKING, jest mi�dzy nie dzielona
ca�a moc procesora dost�pna dla systemu Argante. Jej warto�� mo�na
kontrolowa� ustawiaj�c warto�ci nice oraz schemat schedulingu w
rzeczywistym systemie. 

Mo�liwe jest uruchomienie dw�ch lub wi�cej instancji Argante w
jednym systemie, nale�y jednak wtedy zwr�ci� uwag� na wydajno��,
ewentualnie modyfikuj�� ustawienia multitaskingu w systemie
rzeczywistym i priorytet proces�w Argante.

Je�li tworzony jest system hybrydowy, gdzie Argante wsp�pracuje
z elementami systemu rzeczywistego, sugerowane jest odpowiednie
ustawienie priorytet�w odpowiednio dla Argante i pozosta�ych proces�w
systemu rzeczywistego w celu odpowiedniego podzialu czasu procesora.

Wykonywanie prawid�owo skonstruowanych aplikacji Argante nie powinno
powodowa� zauwa�alnego obci��enia systemu.

9) Przeno�no��

W obecnej chwili, binarne obrazy wykonywalne nie daj� si� przenosi�
mi�dzy platformami o r�nych endianach. Docelowo, w loaderze znajdzie
sie automatyczny translator, obecnie jednak przeno�ny jest tylko kod
�r�d�owy oraz binarki w obr�bie danego endiana.

�r�d�a powinny by� przeno�ne bez ogranicze�.

10) Kompilator si� wyk�ada?

W przypadku gdyby w czasie kompilacji wyst�powa�y problemy typu "memory
exhausted" lub "segmentation fault", nale�y wykomentowa� wszystko w 
linijce CFLAGS= po warto�ci -Wall w pliku Makefile dla danego systemu
(w katalogu sysdep/). Mo�e to spowodowa� nieznaczny spadek wydajno�ci w 
zamian za przyspieszenie czasu kompilacji i zmniejszenie ilo�ci zasob�w 
potrzebnych przy kompilacji.

11) Gdzie zadzia�a Argante?

Linux		- platforma natywna (wspierane readline)
FreeBSD		- przetestowane
NetBSD          - nie przetestowane, powinno dzia�a�
OpenBSD		- przetestowane
Solaris		- przetestowane
AIX             - ??? <je�li masz dost�p, daj zna�>
IRIX            - przetestowane

...inne systemy?


12) Jakie s� szanse na korzystanie z readline?

Ze wzgl�du na niedoskona�o�ci biblioteki readline, a z drugiej strony nasz�
(przynajmniej tymczasow�) niech�� do zajmowania si� kwestiami drugorz�dnymi,
gdy pozostaje do napisania jeszcze wiele istotnego kodu, biblioteka readline
mo�e by� wykorzystywana tylko gdy:

- posiadasz Linuxa
- posiadasz nowe libc6 (glibc 2.1.x).

W innej sytuacji wsparcie dla readline nie zostanie wkompilowane. 

13) Chcia�bym co� napisa� - gdzie jest CVS?

Na razie nie ma CVSa i nic nie wskazuje na to, by mia� powsta� do czasu
wypuszczenia stabilnej wersji systemu. Na razie CVSem jestem ja - 
lcamtuf@ids.pl - i na ten adres powinni�cie nadsy�a� pomys�y lub propozycje,
a tak�e nadsy�a� unified diffs (diff -urN) jesli nanosiliscie jakies
poprawki do kodu. Nie nadsy�ajcie w�asnych snapshot�w ani diff�w uzyskanych
z innymi parametrami - ich r�czne nanoszenie jest do�� uci��liwe.


----------------
99. DO ZROBIENIA
----------------

- IPC/rIPC - to stuka bulba

- math, advmem - z33d zaczyna

- network - kto chce?

- tlumaczenie dokumentacji - robi Artur...

- optymalizacja interpretera bytecode: proponowa�bym zrobi� switch,
  kt�ry ma case obejmuj�ce ca�y dolny dword instrukcji maszynowej
  (wyd�u�y kod, ale go bardzo przyspieszy)... - maxiu sie zglosil

- sko�czenie implementacji hll - to ju� ja, podla praca :)

- do nast�pnej wersji: packet.so - niskopoziomowy dost�p do socket�w

- inne sprytne narz�dzia ;>

-- Micha� Zalewski
