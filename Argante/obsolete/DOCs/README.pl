
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
SPIS TRE¦CI
===========

 1. Wstêp i podziêkowania
 2. Po co powsta³ Argante? Czym siê ró¿ni od innych?
 3. SMTA - model wieloprocesowo¶ci
 4. VCPU - wirtualna architektura
 5. Zestaw poleceñ RSIS
 6. Wyj±tki niskopoziomowe (LLX)
 7. HAC - system kontroli przywilejów
 8. SVFS - architektura systemu plików
 9. IPC/rIPC - komunikacja miêdzyprocesowa <w przygotowaniu>
10. Skrypty i zarz±dzanie konsolowe
11. Pos³ugiwanie siê RSIS-assemblerem
12. Pos³ugiwanie siê translatorem AHLL <w przygotowaniu>
13. Specyfikacja modu³ów standardowych
14. Tworzenie modu³ów
15. Format plików wykonywalnych 
16. Wbudowany debugger
17. FAQ

99. DO ZROBIENIA


-------------------------
 1. Wstêp i podziêkowania
-------------------------

"Argante" jest - stworzonym w du¿ej mierze w przeci±gu kilku dni - wirtualnym
systemem operacyjnym. W obecnej fazie wdra¿ania, ja zajmuje siê wiêkszo¶ci±
rzeczy i nadzorujê rozwój kodu, aczkolwiek mam nadziejê, ¿e wiele innych
osób przy³±czy siê do realizacji tego projektu :)

Dlaczego ten system powsta³ i czym ró¿ni siê od innych rozwi±zañ dowiecie
siê w sekcji 2 tego tekstu, a w miêdzyczasie chcia³bym podziêkowaæ wszystkim,
którzy choæby w najmniejszym stopniu przyczynili siê do obecnego kszta³tu
systemu (celowo pomijam tu ludzi, którzy s± wspó³autorami kodu i zostali
wymienieni wcze¶niej):

  Maja :)                       - za to, ¿e jest i ju¿...
  S³awomir Krawczyk             - za jak zwykle zgry¼liwe uwagi ;)
  Agnieszka S³ota               - za zainteresowanie pomys³em i troskê
  Filip Niedorezo               - za Pierwszy Niezale¿ny Program ;>
  Marek Bia³og³owy              - za "przera¿asz mnie"
  Wojciech Purczyñski           - za mi³± polemikê
  Jarek Sygitowicz              - za plany zaw³adniêcia ¶wiatem
  negativ			- dobre pomys³y :)
  eloy                          - mailing lista :)
  maxiu				- pomys³y, pomys³y na optymalizacje
  Zbyszek Sobiecki		- za network i zainteresowanie :)
  Piotr ¯urawski		- :)
  Lukasz Jachowicz		- za "drukowanie dokumentacji" :>

Ta lista jest do¶æ krótka - mam nadziejê, ¿e siê to zmieni :) Je¶li czujesz
siê na niej pominiêty, napisz mi. Napisz tak¿e je¶li uwa¿asz, ¿e ten projekt
jest ciekawy lub je¶li masz co do niego krytyczne uwagi, pomys³y etc. Ka¿da
opinia jest bardzo cenna. Przede wszystkim jednak istotne jest to, by¶
zapozna³ siê z ca³o¶ci± tego dokumentu, i najpierw spróbowa³ odnale¼æ
odpowiedzi na pytania, które Ci siê nasuwaj±. Dla programistów przyzwyczajonych
do Unixów, do klasycznego assemblera i do tradycyjnych konstrukcji, wiele
rzeczy mo¿e wydaæ siê na pierwszy rzut oka "bezsensowna" - ale zapewniam
Was, ¿e niemal ka¿dy element Argante ma swoje uzasadnienie, i ¿e przy
odrobinie dobrej woli odnajdziecie je tutaj :)

Cenna literatura:

  Steven Muchnick, "Advanced Compiler Design and Implementation"
  Andrew S. Tanenbaum, "Distributed Operating Systems"
  Doreen L. Galli, "Distributed Operating Systems - concepts & practice"
  Andrew S. Tanenbaum, "Modern Operating Systems"
  Eric S. Raymond, "The Cathedral & the Bazaar"
  Illiad, "Evil Geniuses in a Nutshell"

  http://www.vitanuova.com/inferno/papers/bltj.html


  (wiêcej nie mam pod rêk± ;)


----------------------------------------------------
 2. Po co powsta³ Argante? Czym siê ró¿ni od innych?
----------------------------------------------------

Argante jest w pe³ni wirtualnym ¶rodowiskiem uruchamiania aplikacji w
obrêbie systemów Unixowych. Wielu ludziom mo¿e nasuwaæ to skojarzenie np.
z Jav± i jej sandbox'em, aczkolwiek za³o¿enia techniczne, które sta³y siê
podstawami Argante by³y diametralnie ró¿ne.

Przede wszystkim, Argante jest pe³nym systemem operacyjnym. Dysponuje
w³asn± implementacj± procesów, komunikacji miêdzyprocesowej, systemu
plików, modelu uprawnieñ... Po co to wszystko? Postaram siê wyja¶niæ:

Standardowa architektura systemów oraz sprzêtu (np. procesorów) pozostawia 
bardzo wiele do ¿yczenia je¶li chodzi o bezpieczeñstwo i stabilno¶æ
oprogramowania. W skrócie - brak niskopoziomowego wsparcia szeroko pojêtej
kontroli uprawnieñ, obs³ugi b³êdów (prymitywne techniki udostêpniane przez
procesory np. serii 80386 napewno same w sobie nie wystarczaj±), niezbyt 
fortunne za³o¿enia leg³y u podstaw architektury u¿ytkowania segmentów stosu 
czy danych. 

Naprawianie tych b³êdów na wy¿szym poziomie jest na ogó³ niepewne i
ryzykowne. Autorzy Javy doprowadzili do stworzenia ¿a³o¶nie wolnego i w gruncie
rzeczy nie zawsze bezpiecznego rozwi±zania o bardzo ograniczonym zakresie
stosowania, poza tym nie byli w stanie wymusiæ na autorach oprogramowania
korzystania z bezpiecznych, sprawdzonych modeli architektury - jak na przyk³ad
OSI, architektura ograniczonego zaufania i interakcji, która zak³ada, i¿ tylko
najbli¿sze sobie warstwy przetwarzania danych wspó³pracuj± ze sob±, a kod
podzielony jest na segmenty funkcjonalne. Pisane w C programy wykorzystuj±ce
model "listener -> fork() -> client handling" s± wci±¿ prostsze do wdro¿enia
i bardziej niezawodne.

Tak wiêc powsta³a w wyobra¼ni lista tych i wielu innych uwag pod adresem 
popularnego modelu architektury sprzêtowej i programowej. Jej przes³anie
najlepiej wyra¿a motto znajduj±ce siê na pocz±tku tego dokumentu:

"[We] use bad software and bad machines for the wrong things."
					    -- R.W. Hamming

Oprócz tego, poza skargami, nasunê³o mi siê wiele pomys³ów, które moim
zdaniem powinny - i, niewielkim kosztem, mog³yby - zostaæ uwzglêdnione przy
projektowaniu rozwi±zañ na ka¿dym z tych dwóch poziomów - hardware i
software.

Stan±³em w pewnej chwili przed trudn± decyzj±. Mog³em modyfikowaæ istniej±ce 
rozwi±zania, próbuj±c tworzyæ prowizoryczne "³aty" gdzie popadnie, nara¿aj±c 
siê na to, i¿ wiêkszo¶æ pomys³ów nie da siê w pe³ni zrealizowaæ - a tak¿e na 
to, i¿ realizacja projektu stanie siê seri± kompromisów, po¶ród których 
umknie gdzie¶ sens jego realizacji :) Mog³em te¿ zrobiæ co¶ innego - usi±¶æ i 
napisaæ wszystko od pocz±tku. Zapominaj±c o zgodno¶ci, o konwencjach, chc±c 
stworzyæ rozwi±zanie, które bêdzie umia³o obroniæ siê samo, albo na które nikt 
nie zwróci uwagi :) Tak narodzi³ siê pomys³ Argante, z czterema g³ównymi
postulatami:

- bezpieczeñstwo i stabilno¶æ
- funkcjonalno¶æ
- wydajno¶æ
- prostota

Argante mia³ byæ systemem bez kompromisów. Dlatego w sytuacji, gdy w
tradycyjnym rozwi±zaniu stanêliby¶my w obliczu wyboru pomiêdzy bezpieczeñstwem
a funkcjonalno¶ci±, my zamiast wybieraæ jeden z wariantów, dochodzili¶my do
wniosku, i¿ rozwi±zanie jest z³e, i tworzyli¶my jego szkic od podstaw lub
zmieniali¶my model tak, by pogodzi³ stawiane wymagania i nasze oczekiwania.

Dlaczego Argante to system osadzony? Jest kilka powodów. Pierwszy z nich
to to, i¿ rozwi±zanie osadzone nie wymusza zmiany systemu, pozwala na ³atwe
przeprowadzanie prób i wstêpnych wdro¿eñ, w integracji z istniej±cymi
rozwi±zaniami na macierzystej platformie Unixowej. Argante dziêki temu
wprowadza dodatkow± warstwê ochrony i abstrakcji, dzia³aj±c tak, jak zupe³nie
niezale¿na architektura sprzêtowa, nie poci±gaj±c za sob± konieczno¶ci
powa¿nych zmian. Jêzyk C zapewnia z kolei wydajno¶æ i przeno¶no¶æ kodu.
Dziêki temu te¿, implementacja, która mo¿e odwo³ywaæ siê do istniej±cych
sterowników systemowych, urz±dzeñ, funkcji systemowych, sta³a siê du¿o
prostszym zadaniem i pozwoli³a na skupienie siê na aspektach merytorycznych,
a nie detalach implementacyjnych (typu bootloader, sterowniki, etc).

Oczywi¶cie mówi±c o stabilno¶ci i bezpieczeñstwie systemu osadzonego mam
na my¶li to, i¿ implementuje on niezale¿ne od platformy macierzystej
systemy kontroli dostêpu, w³asny model wieloprocesowo¶ci - i wszystkie
te rozwi±zania s± bezpieczne i niezale¿ne od systemu rzeczywistego. Dlatego
Argante na niemal dowolnym Unixie (a mo¿e i Windows?;) bêdzie rozwi±zaniem
bezpiecznym pod warunkiem, i¿ zostanie zapewnione elementarne bezpieczeñstwo
platformy macierzystej - w najprostszym wariancie, usuniête zostan± wszystkie
us³ugi sieciowe (innym tematem s± rozwi±zania hybrydowe, które zostan±
omówione przy okazji rIPC oraz network).

Aby zrealizowaæ wy¿ej wymienione cztery za³o¿enia, stworzy³em ogólne wytyczne
dotycz±ce systemu. Brzmia³y one nastêpuj±co:

- j±dro systemu bêdzie mikrokernelem, zapewniaj±cym podstawow± funkcjonalno¶æ;
  wszystkie operacje wej¶cia/wyj¶cia bêd± odbywa³y siê z wykorzystaniem
  ³adowalnych modu³ów, które mog± byæ w prosty sposób wdra¿ane przez
  u¿ytkownika, a tak¿e dodawane/usuwane w trakcie pracy systemu; modu³y mog±
  tak¿e zawieraæ np. funkcje biblioteczne zapewniaj±ce zaawansowane operacje
  na ³ancuchach tekstowych czy podobne procedury,

- system bêdzie zapewnia³ _dowoln±_ funkcjonalno¶æ, pozwalaj±c± stworzyæ
  w nim oprogramowanie pocz±wszy od serwera baz danych a koñcz±c na aplikacji
  graficznej, bez konieczno¶ci wprowadzania zmian w kodzie systemu, a
  równocze¶nie bêdzie zapewnia³ najwy¿sze bezpieczeñstwo,

- system bêdzie dysponowa³ w³asnym, niskopoziomowym, niezale¿nym od
  platformy sprzetowej jêzykiem maszyny wirtualnej; jêzyk bêdzie wystarczaj±co
  prosty i wydajny, by zapewniæ szybk± i efektywn± pracê, a równocze¶nie
  bêdzie zapewnia³ pe³ne oddzielenie od rzeczywistego systemu i nie bêdzie
  pozwala³ na wykonywanie natywnego kodu maszyny,

- zarz±dzanie systemem bêdzie w pe³ni roz³±czne wzglêdem procesów uruchamianych
  w obrêbie wirtualnego systemu; obowi±zywaæ bêdzie tak¿e pe³na rozdzielno¶æ
  user-space i kernel-space bez mo¿liwo¶ci ingerencji w kernel-space z poziomu
  user-space.

- ka¿dy proces uruchomiony w systemie bêdzie dysponowa³ w³asn±, prywatn±
  przestrzeni± adresow±, wydzielonym segmentem stosu, który nie bêdzie móg³
  byæ bezpo¶rednio adresowany (u¿ywany tylko przez funkcje skoku/powrotu);
  podobnie bêdzie z segmentem kodu, który nie bêdzie bezpo¶rednio adresowalny.
  Tylko segment kodu bêdzie wykonywalny.

- proces bêdzie móg³ alokowaæ bloki pamiêci, roz³±cznie mapowane do jego
  prywatnej przestrzeni adresowej (z mo¿liwo¶ci± ochrony przed zapisem);
  system bêdzie zapewnia³ kontrolê prób naruszenia granic przydzielonego
  bloku (bufora),

- system bêdzie wspiera³ niskopoziomow± obs³ugê wyj±tków i pozwala³ programowi
  je obs³ugiwaæ (LLX - low-level exceptions), 

- system bêdzie posiada³ w³asn±, bezpieczn± i oszczêdzaj±c± zasoby systemowe
  implementacjê multitaskingu oraz w³asny, statyczny model procesów (SMTA)
  z przypisanymi na sta³e grupami przywilejów; wspierane bêd± tak¿e 
  zastosowania wielou¿ytkownikowe przez mo¿liwo¶æ okre¶lania identyfikatora
  podgrupy w obrêbie danej domeny uprawnieñ,

- nowa filozofia przyznawania / zrzucania uprawnieñ, nie nios±ca ze sob±
  ryzyka, którym obarczona jest Unixowa implementacja,

- system od pocz±tku bêdzie wspiera³ bezpieczne rozwi±zania (typu
  unbounded strings w miejsce null-terminated, etc),

- system bêdzie zapewnia³ implementacjê hierarchicznej, scentralizowanej
  i uniwersalnej hierarchicznej kontroli dostêpu (HAC) pozwalaj±cej na
  okre¶lanie przywilejów z dowolnym poziomem szczegó³owo¶ci; dodatkowo,
  system wymusi architekturê "zwrotnicy", wymuszaj±c na programi¶cie
  okre¶lenie, które uprawnienia potrzebne s± do wykonania danej operacji,
  i nie pozwalaj±cego na posiadanie ¿adnych innych,

- system bêdzie silnie wspiera³ architekturê OSI, w tym tak¿e architekturê
  rozproszon±, przez udostêpnienie zaawansowanego mechanizmu komunikacji 
  miêdzyprocesowej IPC (w³asne rozwi±zanie, odmienne od Unixowego) oraz
  rIPC (remote IPC, dystrybuowanie sesji miêdzy to¿samymi procesami,
  transparentna dla user-space komunikacja miêdzy zadaniami na ró¿nych
  komputerach); rIPC bêdzie te¿ wspieraæ transparentn± architekturê
  clustrow±

- system bêdzie zapewnia³ implementacjê w³asnego, wirtualnego systemu
  plików, dostêpnego z poziomu rzeczywistego systemu plików, a równocze¶nie
  pozwalaj±cego na ustalenie dowolnej struktury wewnêtrznej i pe³n± kontrolê
  dostêpu zgodn± z HAC,

- zmiana dowolnego rodzaju funkcjonalno¶ci bêdzie mo¿liwa bez konieczno¶ci
  przerywania pracy systemu

Argante sprzyja tworzeniu rozwi±zañ hybrydowych - na przyk³ad aplikacje
rzeczywistego systemu koordynowane / chronione przez kod Argante. Dziêki
tym mo¿liwo¶ciom bêdzie mo¿na tworzyæ w sposób transparentny redundantnych,
heterogenicznych clustrów z mo¿liwosci± morphingu, samodzielnej przydzielania
nowych obiektow w istniej±cej hierarchii i pe³na redundancja oraz load
balancingiem bez _¿adnych_ nak³adów programistycznych. Nie ma ró¿nicy,
czy system bêdzie pracowa³ na jednej maszynie, czy na stu, z reduntantnymi
rozwi±zaniami i load balancingiem - po prostu filozofia rIPC rozwi±zuje
problemy systemów rozproszonych w sposób przejrzysty dla aplikacji.

Wiem, ¿e brzmi to jak pobo¿na lista ¿yczeñ, ale piszê te s³owa ju¿ po
wdro¿eniu wiêkszo¶ci kodu systemu i ze zdumieniem (nieskromnie) stwierdzam, ¿e 
uda³o mi siê osi±gn±æ te zamierzenia. Co uzyska³em?

- bezpieczeñstwo i stabilno¶æ: 

  - praktycznie brak mo¿liwo¶ci przejêcia kontroli nad aplikacj± w systemie
    (kontrola stosu, segmentu danych, buforów, filozofia przekazywania
    parametrów do syscalli nie wymuszona konwencjami C - typu null-term);
    ze wzglêdu na niewielki zestaw poleceñ maszynowych RSIS, kontrola
    uprawnieñ jest aspektem banalnym,

  - sprzyjanie programowaniu zgodnemu z bezpieczn± architektur± OSI -
    po prostu jest to intuicyjne w tym systemie,

  - wymuszanie kontroli poprawno¶ci wykonywania kodu przez zg³aszanie
    wyj±tków,

  - nawet je¶li by³oby to mo¿liwe, brak mo¿liwo¶ci zdobycia uprawnieñ 
    pozwalaj±cych na naruszenie bezpieczeñstwa reszty systemu wirtualnego
    (oddzielenie zarz±dzania od systemu wirtualnego, odciêcie od kernel-pace),

  - a nawet gdyby to by³o mo¿liwe, niemo¿no¶æ wp³yniêcia na pracê
    rzeczywistego systemu (oddzielna implementacja multitaskingu, nie
    wykorzystuj±ca implementacji systemu rzeczywistego),

  - pe³na kontrola dostêpu do dowolnych zasobów (HAC), wspomniana ju¿
    nowa filozofia uprawnieñ, nowa filozofia wi±zania przywilejów z
    procesem, i nowy model procesów, etc...

  - destabilizacja systemu macierzystego jest w³a¶ciwie niemo¿liwa,

  - wsparcie redundancji i dystrybucji zapytañ,

- funkcjonalno¶æ i prostota:

  - uniwersalno¶æ systemu przez zapewnienie wsparcia wygodnych modu³ów
    i scentralizowanej kontroli, oraz efektywnej architektury wirtualnego
    procesora o niewielkim, lecz wydajnym zestawie poleceñ,

  - mo¿liwo¶æ tworzenia systemów rozproszonych bez konieczno¶ci modyfikacji
    kodu; mo¿liwo¶æ propagowania requestów bez konieczno¶ci modyfikacji
    kodu (mowa o kodzie aplikacji),

  - wyj±tki upraszczaj± obs³ugê b³êdów,

  - wprowadzanie nawet powa¿nych zmian do systemu mo¿e odbywaæ siê w locie
    przez podmianê modu³ów,

  - load balancing, tworzenie clustrów, rozdzielanie rozwi±zania miêdzy maszyny
    mo¿e odbywaæ siê bez modyfikacji kodu ¼ród³owego jego elementów,

- wydajno¶æ:

  - dziêki u¿yciu niskopoziomowego kodu wirtualnego, zamiast - jak np.
    w przypadku Javy - kodu wysokopoziomowego - spadek wydajno¶ci nie jest
    tak ra¿±cy, nie ogranicza siê te¿ mo¿liwo¶ci kodu. Pêtle typu
    "idle loop" (tzn powtarzany w kó³ko skok) wykonywa³ siê kilkakrotnie
    wolniej, ni¿ skompilowany w C na natywnej platformie sprzêtowej, co jest
    wynikiem bardzo dobrym. W przypadku bardziej z³o¿onych operacji (np. I/O),
    spadek wydajno¶ci jest znacznie mniejszy i waha siê w zakresie 15-30%.

  - zaimplementowany multitasking jest du¿o bardziej stabilny i mniej
    pamiêcio¿erny ni¿ w przypadku natywnego systemu; wynika to po czê¶ci
    z tego, ¿e wirtualny procesor Argante potrzebuje mniej informacji
    by "utrzymaæ" proces ni¿ Unix, a po czê¶ci z niedoskona³o¶ci wielu
    systemów.

Chcieli¶my po³±czyæ QNXa, HURDa, oraz wszystkie nasze "lu¼ne" pomys³y by
stworzyæ naprawde bezpieczne i efektywne rozwi±zanie :) Pó¼niej, Pawe³
Krawczyk zwróci³ nam uwagê na system osadzony Inferno, który choæ ró¿ni
siê od Argante, ma te¿ z ni± wiele wspólnego. Informacje o nim znajdziecie
pod adresem http://www.vitanuova.com/inferno/papers/bltj.html. System
by³ wdra¿any przez Lucenta.

Wierzymy, ¿e unikneli¶my wielu niepe³nych rozwi±zañ, jak na przyk³ad
przenoszenie wysokopoziomowej funcjonalno¶ci na ni¿sze poziomy bez dobrego
powodu; decydowali¶my siê na takie posuniêcie bardzo rzadko, tylko gdy
byli¶my pewni i¿ zaoferuje to rzeczywist± korzy¶æ dla systemu, bez
narzucania statycznych, z³o¿onych rozwi±zañ gdzie nie s± one potrzebne.

Uff, to tyle. Pora skoñczyæ z marketingiem i przej¶æ do szczegó³ów
implementacyjnych ;)

Aha, do¶æ prosty, ale radosny tutorial uruchamiaj±cy dwa programy
mo¿na obejrzeæ wpisuj±c "./build test".


----------------------------------
 3. SMTA - model wieloprocesowo¶ci
----------------------------------

Koncepcja procesów w Argante mo¿e dla wielu osób wydaæ siê szokuj±ca, zw³aszcza
je¶li przywykli do Unixowego schematu, który zak³ada: jeden klient, jeden
proces. W Argante procesy s± obiektami statycznymi - zostaj± powo³ane do
¿ycia z poziomu konsoli zarz±dzaj±cej lub skryptu. Przynajmniej wed³ug
standardowej semantyki Argante (oczywi¶cie nic nie stoi na przeszkodzie, by
j± zmieniæ, dodaj±c nowy syscall), procesy nie maj± mo¿liwo¶ci siê mno¿yæ,
tworzyæ potomstwa lub wykonywaæ w swoje miejsce inne programy.

Zamiast tego, Argante wspiera model OSI, gdzie procesowi przypisany jest
nie obiekt (jak na przyk³ad ³±cz±cy siê klient), ale pewna funkcja (np.
obs³uga bazy danych czy obs³uga po³±czeñ). Choæ wydaje siê to byæ dodatkowym
utrudnieniem, jestem pewien, i¿ gdy dotrzecie do koñca tego dokumentu,
przekonacie siê, i¿ nie jest to co¶ z³ego. Proces jest wczytywany w
przestrzeñ wirtualnego procesora - VCPU - i tam egzystuje, dopóki nie 
zakoñczy swojej pracy, lub nie przytrafi siê krytyczny b³±d (typu
nieobs³u¿ony wyj±tek).

Wiêkszo¶æ parametrów procesu - jak na przyk³ad priorytet, czy te¿ zestaw
"domen" (które s± obiektem zbli¿onym to Unixowych supplementary groups)
jest przypisany do obrazu binarnego danego pliku wykonywalnego w chwili
kompilacji. Oto przyk³adowa struktura demona ftp, który spe³nia wymogi OSI
i jest bardzo prosty do zaimplementowania w Argante (byæ mo¿e prostszy
ni¿ w C), a do tego zdecydowanie bardziej bezpieczny i... wydajniejszy:

  TCP/IP                                         baza danych
    |                                                 |           "reality"
  --|-------------------------------------------------|----------------------
  (net) (ipc)-------(ipc)-(ipc)-(ipc)   (ipc)-(ipc)  (fs)       kernel space
  --|-----|-----------|-----|-----|-------|-----|-----|----------------------
    |     |           |     |     |       |     |     |           user space
   <A>----+          <B>   <B>   <B>------+    <C>----+

A - proces obs³uguj±cy po³±czenia z sieci - odbiera po³±czenia,
    ³±czy siê z jednym z procesów B po IPC i przekazuje im polecenia

B - procesy obs³uguj±ce klienta (dowolna ilo¶æ, automatyczna propagacja
    zleceñ); obs³uguj± polecenia, po IPC komunikuj± siê z procesem
    autoryzuj±cym; dziêki ³atwo¶ci u¿ycia IPC i wsparciu kontekstów
    obs³uga wielu sesji w jednym procesie nie jest problemem.

C - proces realizuj±cy autoryzacjê; korzystaj±c z modu³u fs weryfikuje
    wpisy w lokalnej bazie danych dostêpnej w SVFS.

To, jak realizowana jest komunikacja miêdzy procesami, jak dzia³aj±
uprawnienia i zmiana grup, powiemy sobie przy omawianiu systemu HAC. Na
razie wypada nadmieniæ, i¿ procesy nie bêd± mia³y nigdy równocze¶nie
mo¿liwo¶ci np. wykonywania operacji z wykorzystaniem modu³u net i ipc
(wspomniana wcze¶niej "zwrotnica"), ani ¿e proces A nie bêdzie móg³ po IPC
nawi±zaæ po³±czenia bezpo¶rednio z C.

Po pierwsze, bezpieczeñstwo. Po drugie, z³o¿ono¶æ programistyczna na
poziomie nie przekraczaj±cym napewno tej samej aplikacji w C. Po trzecie,
niewspó³miernie wiêksza wydajno¶æ w porównianiu z modelem wykorzystuj±cym
fork().

Jak dzia³a multitasking? Generalnie, jest do¶æ sprawiedliwy. Ka¿demu procesowi
w danym cyklu obs³ugi procesów przys³uguje tyle cykli maszynowych jego
wirtualnego procesora, ile wynosi warto¶æ "priorytetu" procesu. Tak wiêc
proces o priorytecie 10000 otrzyma 10000 cykli, natomiast proces o priorytecie
1 - jeden cykl. Zalecane jest oczywi¶cie nadawanie procesom rozs±dnych
warto¶ci priorytetu z zakresu 100-10000.

Procesy mog± znale¼æ siê w stanie STATE_SLEEPFOR, w którym ich wykonywanie
jest wstrzymywane na okre¶lon± ilo¶æ cykli obs³ugi procesów; mo¿liwy jest
te¿ stan STATE_SLEEPTILL, gdy proces oczekuje okre¶lon± ilo¶æ mikrosekund,
lub STATE_IOWAIT, gdy proces oczekuje np. na otrzymanie prawa zapisu do
pliku, do którego w danej chwili zapisuje inny proces, lub na odebranie
danych z socketa (oczywi¶cie tylko je¶li za¿yczy sobie zostaæ wprowadzony
w ten stan, gdy¿ mo¿e te¿ wywo³aæ funkcjê z opcj± NONBLOCK).


---------------------------------
 4. VCPU - wirtualna architektura
---------------------------------


Technicznie rzecz bior±c, VCPU jest wirtualn± maszyn±, udostêpniaj±c±
niewielki, lecz wygodniejszy w u¿yciu od tradycyjnego kodu maszynowego
zestaw instrukcji, trzy bloki rejestrowe (odpowiednio do operacji na
32-bitowych warto¶ciach ca³kowitych bez znaku, ze znakiem, oraz dla
liczb zmiennoprzecinkowych) po 16 rejestrów, przestrzeñ stosu (wykorzystywan±
wy³±cznie przy wywo³aniach funkcji; do przechowywania danych s³u¿± inne
rozwi±zania), numer identyfikacyjny w komunikacji miêdzyprocesowej IPC/RIPC,
przestrzeñ wykonywania programu, przestrzeñ przeznaczon± na alokacjê bloków 
pamiêci, z implementacj± mechanizmu kontroli dostêpu i dynamicznej
realokacji, oraz kilka innych, mniej istotnych zmiennych. Alokowane bloki 
pamiêci, które mog± byæ u¿ywane do przechowywania i obróbki danych, s± 
roz³±czne i nie jest mo¿liwe przepisanie innego bloku przy przekroczeniu 
granicy poprzedniego. Nie jest tak¿e mo¿liwe modyfikowanie stosu oraz kodu 
programu, o czym wspomina³em wcze¶niej.

Adresowanie pamiêci alokowalnej odbywa siê nie jak w standardowych systemach
w systemie 8-bitowym, ale z wykorzystaniem dwords, 32-bitowych skoków. Oznacza
to wy¿sz± wydajno¶æ w wiêkszo¶ci zastosowañ, i przy okazji bezpieczniejszy
dostêp do danych.

Podobnie z adresowaniem przestrzeni kodu. Ka¿da instrukcja kodowana jest
za pomoc± 12 bajtów. Instrukcje, które nie potrzebuj± argumentów, jak np. NOP,
maj± ustawiony tylko pierwszy bajt oznaczaj±cy opcode. W innym przypadku,
kolejne dwa bajty oznaczaj± rodzaj parametru, jeden bajt jest "paddingiem",
oraz dwa kolejne bajty zawieraj± parametry. W³a¶ciwie mo¿na zamkn±æ siê w
10 bajtach, ale tracimy wtedy na wydajno¶ci (w obecnym uk³adzie mamy trzy
razy 32-bitowy dword).

Takie rozwi±zanie pozwala na bezpieczniejsze poruszanie siê w obrêbie
segmentu kodu (tak¿e przy skokach) oraz na zmniejszenie ilo¶ci opcodów, a
tak¿e na drastyczny zysk je¶li chodzi o ilo¶æ instrukcji koniecznych do
wykonania danej operacji, co kompensuje rozmiar pojedynczej instrukcji:

       1     2     3     4     5     6     7     8     9     10    11    12
 +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
  xxxxx xxxxx xxxxx RSRVD xxxxxxxxxxxxxxxxxxxxxxx xxxxxxxxxxxxxxxxxxxxxxx
  |     |     |           |                       |
  |     |     |           +-----------------------+- dwa parametry 32-bitowe
  |     |     +------------------------------------- typ drugiego parametru
  |     +------------------------------------------- typ pierwszego parametru
  +------------------------------------------------- opcode, np. MOV

Typy parametrów to:

IMMEDIATE - 32-bitowa warto¶æ liczbowa
UREG      - numer rejestru typu unsigned
SREG      - numer rejestru typu signed
FREG      - numer rejestru typu float
IMMPTR    - wska¼nik liczbowy do 32-bitowej warto¶ci liczbowej
UPTR      - numer rejestru typu unsigned zawieraj±cy wska¼nik do 32-bitowej
            warto¶ci liczbowej


Uwaga: w przypadku instrukcji skoku, podanie parametru typu IMMEDIATE
czy UREG oznacza po prostu adres. W przypadku instrukcji typu MOV jest
inaczej - je¶li chcemy odwo³aæ siê do adresu miejsca w pamiêci, powinni¶my
pos³u¿yæ siê typem IMMPTR lub UPTR. Jest to umowna konwencja, pozwalaj±ca
na efektywniejsze u¿ycie komend.

Dostêpne s± nastêpuj±ce rejestry:

  u0 .. u15             - rejestry typu unsigned, bez znaku (0..15)
  s0 .. s15             - rejestry typu signed, ze znakiem  (100..115)
  f0 .. f15             - rejestry zmiennoprzecinkowe (200..215)

System zapewnia konwersjê przy operacjach na rejestrach, aczkolwiek jest
to zabieg czasoch³onny i nie powinien byæ u¿ywany zbyt czêsto. Nie jest
przeprowadzana konwersja warto¶ci pobieranych z pamiêci (a wiêc zapisuj±c
pod adres 1234 warto¶æ rejestru f0 = 0.123, a potem odczytuj±c warto¶æ z
tego adresu do rejestru u0, otrzymamy prawdopodobnie nieporz±dany wynik;
rozwi±zaniem jest wczytanie warto¶ci ponownie do rejestru f0 i u¿ycie
mov u0,f0).

Proces uruchomiony na VCPU mo¿e pos³ugiwaæ siê wy³±cznie natywnym zestawem
instrukcji, okre¶lonymi jako RSIS, bez mo¿liwo¶ci wykonywania bezpo¶rednio
kodu maszynowego rzeczywistego procesora.


-----------------------
 5. Zestaw poleceñ RSIS
-----------------------

Oto wstêpny opis poleceñ maszynowych systemu:


Mnemonik:  NOP
Parametry: -
Opcode:	   0
Opis:	   nie rób nic
Efekt:     -
Wyj±tki:   -


Mnemonik:  JMP <addr>
Parametry: <addr> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   1
Opis:	   skok bezwarunkowy pod adres absolutny
Efekt:	   zmiana IP
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  IFEQ <x> <y>
Parametry: <x> = dowolny, <y> = dowolny
Opcode:	   2
Opis:	   wykonanie nastepnego polecenia je¶li <x> = <y>
Efekt:	   warunkowo zmiana IP
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  IFNEQ <x> <y>
Parametry: <x> = dowolny, <y> = dowolny
Opcode:	   3
Opis:	   wykonanie nastepnego polecenia je¶li <x> != <y>
Efekt:	   warunkowo zmiana IP
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  IFABO <x> <y>
Parametry: <x> = dowolny, <y> = dowolny
Opcode:	   4
Opis:	   wykonanie nastepnego polecenia je¶li <x> > <y>
Efekt:	   warunkowo zmiana IP
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  IFBEL <x> <y>
Parametry: <x> = dowolny, <y> = dowolny
Opcode:	   5
Opis:	   wykonanie nastepnego polecenia je¶li <x> < <y>
Efekt:	   warunkowo zmiana IP
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  CALL <addr>
Parametry: <addr> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   6
Opis:	   skok bezwarunkowy pod adres absolutny z od³o¿eniem adresu
Efekt:	   zmiana IP, od³o¿enie adresu na stos
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT, STACK_OVER


Mnemonik:  RET <cnt>
Parametry: <cnt> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   7
Opis:	   powrót pod <cnt> adres zdjêty ze stosu
Efekt:	   zmiana IP, pobranie ze stosu
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT, STACK_UNDER


Mnemonik:  HALT
Parametry: -
Opcode:	   8
Opis:	   zakoñczenie pracy VCPU; tak¿e w trybie respawn
Efekt:	   -
Wyj±tki:   -


Mnemonik:  SYSCALL <nr>
Parametry: <nr> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   9
Opis:      wywo³anie syscalla
Efekt:	   zale¿ny od syscalla
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT, NOMODULE + 
           zale¿ne od syscalla


Mnemonik:  ADD <x> <y> - opcode 10
           SUB <x> <y> - opcode 11
           MUL <x> <y> - opcode 12
           DIV <x> <y> - opcode 13
           MOV <x> <y> - opcode 19
Parametry: <x> = UREG, FREG, SREG, IMMPTR, UPTR
           <y> = IMMEDIATE, UREG, FREG, SREG, IMMPTR, UPTR
Opis:      operacje arytmetyczne (+, -, *, /, przypisanie)
Efekt:	   zmiana warto¶ci pierwszego argumentu
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  MOD <x> <y> - opcode 14
           XOR <x> <y> - opcode 15
	   REV <x> <y> - opcode 16
           AND <x> <y> - opcode 17
           OR  <x> <y> - opcode 18
Parametry: <x> = UREG, SREG, IMMPTR, UPTR
	   <y> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opis:	   operacje binarne
Efekt:	   zmienia warto¶æ pierwszego argumentu
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  CWAIT <x>
Parametry: <x> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opcode:    20
Opis:      usypia proces na <x> taktów SMTA
Efekt:     -
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  TWAIT <x>
Parametry: <x> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opcode:    21
Opis:      usypia proces na [conajmniej] <x> mikrosekund
Efekt:     -
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  ALLOC <size> <prot>
Parametry: <size> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           <size> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opcode:    22
Opis:      alokuje blok pamiêci o rozmiarze size i flagach uprawnieñ
	   prot.
Efekt:     u0 - numer identyfikacyjny bloku, u1 - adres pocz±tkowy bloku
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT, NOMEM


Mnemonik:  REALLOC <nr> <size>
Parametry: <nr> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           <size> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opcode:    23
Opis:      realokuje blok pamiêci o numerze nr tak by mia³ rozmiar size.
Efekt:     -
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT, NOMEM


Mnemonik:  DEALLOC <nr>
Parametry: <nr> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
Opcode:    24
Opis:      dealokuje blok pamiêci o numerze nr
Efekt:     -
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  CMPCNT <addr1> <addr2>
Parametry: <addr1> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           <addr2> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           u0 - liczba dwords
Opcode:    25
Opis:      porównuje <addr1> i <addr2> na przestrzeni u1 bajtów
Efekt:     u0 - 0 = porównanie pomy¶lne, !0 - niezgodno¶æ
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  CPCNT <addr1> <addr2>
Parametry: <addr1> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           <addr2> = IMMEDIATE, UREG, SREG, IMMPTR, UPTR
           u0 - liczba dwords
Opcode:    26
Opis:      przepisuje <addr2> do <addr1> na przestrzeni u1 bajtów
Efekt:     -
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  ONFAIL <addr>
Parametry: <addr> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   27
Opis:	   skok w razie wyj±tku pod adres absolutny; anulowane po RET
           poni¿ej obecnego poziomu wykonywania.
Efekt:	   -
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  NOFAIL
Parametry: -
Opcode:	   28
Opis:	   anulowanie ONFAIL na obecnym poziomie wykonywania
Efekt:	   -
Wyj±tki:   -


Mnemonik:  LOOP <addr>
Parametry: <addr> = IMMEDIATE, UREG, IMMPTR, UPTR
           s0 - licznik pêtli
Opcode:	   29
Opis:	   skok je¶li s0 jest wiêksze od zera pod adres absolutny,
           zmniejszenie s0 o jeden
Efekt:	   zmiana IP, zmiana s0
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Mnemonik:  RAISE <nr>
Parametry: <nr> = IMMEDIATE, UREG, IMMPTR, UPTR
Opcode:	   30
Opis:	   zg³oszenie wyj±tku <nr>
Efekt:	   zg³oszenie wyj±tku
Wyj±tki:   OUTSIDE_REG, BAD_PARAM, OUTSIDE_MEM, PROTFAULT


Jak zapewne zauwa¿yli¶cie, zarz±dzanie pamiêci± nie zosta³o umieszczone
w oddzielnym module i jest integraln± czê¶ci± systemu. Jest to wyj±tek,
podyktowany wzglêdami wydajno¶ci oraz funkcjonalno¶ci, nic oczywi¶cie nie
przeszkadza w stworzeniu dowolnie wyrafinowanego systemu zarz±dzania
pamiêci± z wykorzystaniem syscalli.

Kolejna istotna uwaga - kilka osób narzeka³o, i¿ ograniczony zakres
instrukcji RSIS nie pozwala na np. efektywne operacje graficzne czy generalnie
wyszukane dzia³ania na pamiêci. W RSIS - podobnie jak choæby w C - podstawowy
jêzyk zapewnia tylko najprostsze operacje oraz konstrukcje pozwalaj±ce
sterowaæ wykonywaniem etc. Natomiast wszystkie zaawansowane funkcje - jak
choæby memfrob() lub podobne - s± funkcjami nie bêd±cymi elementami jêzyka,
a elementami bibliotek. W Argante mo¿na napisaæ choæby modu³ advgraph.c,
który bêdzie odpowiada³ za z³o¿one operacje na obiektach graficznych i
komunikacjê z kart±, ale nie nale¿y oczekiwaæ tego od RSIS (nie bêdzie
wiêc wydania Argante MMX ;). Choæ modu³ obs³ugi grafiki nie jest w tej
chwili przewidywany, wiele funkcji pomocnych w obróbce bloków pamiêci
oraz danych tekstowych znajdzie siê w module advmem.c.



--------------------------------
 6. Wyj±tki niskopoziomowe (LLX)
--------------------------------

Wyj±tki zg³aszane przez polecenia (mog± byæ obs³u¿one ONFAIL):

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

Przyjêt± polityk± jest informowanie (u¿ywaj±c funkcji non_fatal())
programu za pomoc± zg³aszanych wyj±tków o sytuacjach nietypowych /
alarmowych. Tak wiêc syscall sprawdzaj±cy istnienie pliku w razie jego
nieobecno¶ci nie powinien zg³aszaæ wyj±tku. Natomiast syscall s³u¿±cy
do otworzenia pliku w tej sytuacji powinien go zg³osiæ.

W chwili wywo³ania handlera ONFAIL w u0 znajduje siê numer wyj±tku.
Po wykonaniu ret przywracana jest pierwotna zawarto¶æ u0. O ewentualne
zapisanie reszty modyfikowanych rejestrów handler powinien zatroszczyæ siê
oczywi¶cie sam.

Jak dzia³aj± wyj±tki? Na ka¿dym poziomie wykonywania, mo¿emy zg³osiæ
handler. Np. jeste¶my w g³ównym kodzie (stack ptr == 0), i za pomoc± ONFAIL
deklarujemy handler wyj±tków. Nastêpnie wywo³ujemy funkcjê lokaln±, która
ma w³asny ONFAIL - wtedy wyj±tki nie bêd± "przekazywane" do g³ównego
poziomu wykonywania. Je¶li potem wrócimy z funkcji lokalnej i wywo³amy
inn±, bez oddzielnego ONFAIL, w razie wyst±pienia wyj±tku funkcja (i
wszystkie pó¼niej wywo³ane) zostan± "puszczone w niebyt", i program powróci
do g³ównego w±tku, wywo³uj±c to, co zadeklarowano jako ONFAIL na tym
poziomie wykonywania.


-------------------------------------
 7. HAC - system kontroli przywilejów
-------------------------------------


Stworzony jest zunifikowany mechanizm zarz±dzania przywilejami (HAC).
Przyk³adowy wpis w pliku access.set, który jest plikiem konfiguracyjnym
podsystemu:

12345:00000     fs/ftp/uzytkownicy      fs/fops/new/dir         allow
|               |                       |                       |
| +-------------+                       |      znaczenie wpisu _|
| | +-----------------------------------+     - allow lub deny
| | |
| | +-- hierarchiczny identyfikator rodzaju dostêpu: przestrzeñ uprawnieñ
| |     to 'fs', ga³±¼ operacji na plikach (fops), rodzaj operacji: tworzenie
| |     obiektów (new), rodzaj obiektu: katalog (dir). Jest to konwencja
| |     sugerowana, tak jak mówi³em kernel nie odpowiada za autoryzacjê.
| |     Robi± to modu³y, przekazuj±c dane funkcji is_permitted().
| |
| +---- hierarchiczny identyfikator zasobu; w tym przypadku opisana jest
|       przestrzeñ obiektu, przestrzeñ systemu plików (pliki ftp),
|       oraz konkretny katalog.
|
+------ przynale¿no¶æ do grupy; warto¶æ '0' oznacza, i¿ regu³a ma charakter
        "generic" i odnosi siê do wszystkich grup; warto¶æ po znaku ':'
        oznacza podgrupê. Tu zgodno¶æ, w przypadku regu³ specyfikuj±cych
        niezerow± grupê, musi byæ pe³na.

Wpisy s± honorowane w kolejno¶ci wystêpowania w pliku konfiguracyjnym.
Dlatego bardziej specyficzne wpisy (np. zawieraj±ce odmowê dostêpu do zasobu
dla danej podgrupy) powinny znale¼æ siê przed ogólniejszymi.

UWAGA: Je¶li identyfikator operacji to w pliku konfiguracyjnym to np.
'fs/fops', oznacza to, i¿ osobnik podpadaj±cy pod pozosta³e kryteria,
a chc±cy dostêpu np. do 'fs/fops/new/file/text', otrzyma te uprawnienia.
Oczywi¶cie nie dzia³a to w drug± stronê i wpis 'fs/fops/new/file/text'
nie implikuje dostêpu do ca³ej hierarchii 'fs/fops'. U¿ywanie '/'
jako separatorów jest konieczne - np wpis 'fs_ops' nie oznacza dostêpu
do obiektu 'fs_ops_new_file'.

HAC narzuca regu³ê precyzowania operacji w stronê u¶cislania ich typu
obiektu, na którym operacja jest wykonywana. Tak wiêc _ZAWSZE_ poprawnym
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

Natomiast NIEPOPRAWNYM zapisem jest na przyk³ad fs/fops/file/delete,
fs/fops/file/create, etc. Choæ w wielu sytuacjach na pierwszy rzut oka mo¿e siê
to wydaæ nielogiczne, to jednak drugi zapis praktycznie uniemo¿liwi³by
generalizowanie regu³ (np. danie uprawnieñ do tworzenia obiektów w
danej czê¶ci filesystemu to wpisanie fs/fops/create, podczas gdy w drugiej
notacji oznacza³oby to wiele wpisów).

Aby chroniæ siê przed próbami wprowadzenia w b³±d modu³ów obs³uguj±cych
filesystem, system autoryzacji odmawia dostêpu do obiektów zawieraj±cych
ci±g "/..". Ich eliminacj± powinien zaj±æ siê modu³ (i zajmuje siê).

W przypadku podsystemów, w przypadku których nie da siê okre¶liæ zasobu,
b±d¼ jego okre¶lanie by³oby dublowaniem rodzaju dostêpu (np. modu³
wy¶wietlaj±cy tekst na wirtualnej konsoli - okre¶lenie rodzaju operacji
jest tu wystarczaj±ce), jako zasób nale¿y podaæ "none".

Testowanie regu³ek przed ich aktualizacj± mo¿e odbyæ siê z u¿yciem
do³±czonego programu 'actest' (w katalogu tools), który zapewnia
do¶æ solidn± diagnostykê HAC. Aktualizacja regu³ek - polecenie '^' na
konsoli zarz±dzaj±cej (patrz dalej).

Je¶li nie wiesz jeszcze nic o modu³ach, mo¿esz wróciæ tu pó¼niej. Poni¿ej
znajduje siê interfejs HAC dla modu³ów:


Z punktu widzenia autora modu³u, najwygodniejszym interfejsem do
kontroli dostêpu jest makro VALIDATE() z pliku include/acman.h.
Makro akceptuje trzy parametry: numer procesora, identyfikator zasobu
i identyfikator typu dostêpu.

Na przyk³ad:

  VALIDATE(c,"net/tcp/destination/10.0.0.1/1234","net/connect");

W przypadku, gdy dostêp jest mo¿liwy, makro nie odniesie efektu. W razie
odmowy dostêpu, makro zg³osi wyj±tek NOPERM z opisem sytuacji i wyjdzie
z funkcji syscall_handler(), z której powinno byæ wywo³ane.

W celu obs³ugi w bardziej finezyjny sposób, mo¿na korzystaæ z funkcji
is_permitted(), akceptuj±cej parametry zgodne z parametrami makra
VALIDATE(), ale zwracaj±cego warto¶ci 0 (odmowa) lub 1 (dostêp mo¿liwy).
Nie jest zg³aszany wyj±tek ani nie ma miejsca return z funkcji. Dla
¶cis³o¶ci, makro VALIDATE() jest zbudowane w nastêpuj±cy sposób jako
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

Modu³ powinien wysy³aæ mo¿liwie jak najbardziej dospecyfikowane zapytanie,
zawieraj±ce komplet danych niezbêdnych do weryfikacji uprawnieñ. Modu³
obs³uguj±cy grafikê nie powinien wiêc pytaæ o 'graph' tylko o
'graph/control/setmode' i o zasob 'graph/res/640/480/16bpp'. Podobnie w
pliku konfiguracyjnym access.hac nale¿y mo¿liwie dok³adnie zawê¿aæ
regu³y.


--------------------------------------
 8. SVFS - architektura systemu plików
--------------------------------------

System plików definiowany jest w pliku conf/fsconv.dat. Zawiera on
oddzielony spacjami model mapowania wirtualnego systemu plików na system
rzeczywisty. Regu³y doboru s± podobne jak w przypadku HAC:

fs/ftp/test1            /Argante/fs/inny_katalog
fs/ftp                  /Argante/fs/serwer_ftp

HAC kontroluje dostêp na poziomie wirtualnych katalogów. Powy¿sze wpisy
oznaczaj±, ¿e katalog fs/ftp/test1 jest mapowany w inne miejsce, ni¿ katalog
fs/ftp. Je¶li jaki¶ proces bêdzie posiada³ wpis HAC pozwalaj±cy na
operacjê typu fs/create/directory na obiekcie fs/ftp, bêdzie posiada³ dostêp
do obu katalogów (zgodnie z za³o¿eniami HAC, o ile nie zostanie to
wcze¶niej wykluczone). Tworz±c katalog fs/ftp/nope, rzeczywisty wpis
powstanie w miejscu /Argante/fs/serwer_ftp/nope. Natomiast ta sama operacja
dla fs/ftp/test1/nope, zaowocuje plikiem /Argante/fs/inny_katalog/nope.
Natomiast próba dostêpu np do obiektu fs/ftp/../nope skoñczy siê
niepowodzeniem - modu³ systemu plików rozpozna j± jako dostêp do obiektu
fs/nope, tymczasem taki wpis nie istnieje w hierarchii SVFS.

Filozofia systemu plików Argante zak³ada jednocze¶nie niezale¿n± od
rzeczywistego systemu kontrolê dostêpu do zasobów i ochronê systemu
macierzystego, a równocze¶nie mo¿liwo¶æ integracji systemu plików SVFS
z obiektami rzeczywistego systemu plików.

System SVFS jest mo¿liwie uproszczonym, lecz w pe³ni funkcjonalnym
podzbiorem operacji na systemu plików. W pierwotnej wersji nie posiada
wsparcia dla tworzenia symlinków lub hardlinków, aczkolwiek wspiera te
istniej±ce na poziomie systemu rzeczywistego.

W³±czanie istotnych zasobów / katalogów systemowych bezpo¶rednio w
hierarchiê SVFS jest mo¿liwe (np. udostêpnienie katalogu /etc), ale
zdecydowanie odradzane.


------------------------------------------------------------
 9. IPC/rIPC - komunikacja miêdzyprocesowa <w przygotowaniu>
------------------------------------------------------------

Je¶li chcia³by¶ zaj±æ siê wdro¿eniem systemu IPC - daj znaæ!

Modu³ ten nie jest jeszcze wdro¿ony - poni¿ej znajduje siê wczesny draft:

W komunikacji miêdzyprocesowej warunkiem jest to, i¿ dwa procesy chc±ce
otworzyæ miêdzy sob± sesjê [r]IPC, musz± przynale¿eæ przynajmniej do jednej
wspólnej grupy (lub podgrupy, je¶li ta zosta³a ustawiona), a tak¿e musz±
wyraziæ chêæ nawi±zania sesji. Oczywi¶cie mo¿liwa jest tak¿e kontrola
mo¿liwo¶ci korzystania z mechanizmu IPC w ogóle za pomoc± HAC.

Ka¿dy proces, który chce korzystaæ z IPC, musi zarejestrowaæ swój numer
identyfikacyjny na potrzeby komunikacji i znaæ numer identyfikacyjny
drugiej strony (mo¿liwe jest nadanie dwóm lub wiêcej procesom tego samego
numeru w celu implementacji dystrybucji sesji IPC).

Zasad± funkcjonowania IPC jest potwierdzanie ka¿dego dzia³ania - tak wiêc
je¶li jedna strona, po nawi±zaniu sesji IPC (handshake) za¿yczy sobie
skopiowania okre¶lonego obszaru pamiêci z przestrzeni jednego procesu
do przestrzeni drugiego procesu, druga strona musi odebraæ request i
wys³aæ potwierdzenie do modu³u IPC.

Dziêki statycznym parametrom procesów, mo¿liwa jest jednoznaczna identyfikacja
programu zg³aszaj±cego request i jednoznaczne okre¶lenie odbiorcy, dane te
s± jawne dla obu stron (udostêpniane przez modu³). Proces wygl±da tak samo,
niezale¿nie od tego, czy procesy dzia³aj± w obrêbie jednego systemu fizycznego,
czy systemów rozproszonych w sieci. W tym drugim przypadku po³±czenie
systemów IPC na dwóch hostach musi zostaæ zainicjowane przez operatora lub
z poziomu skryptów nadzorczych.

Oczekiwana funkcjonalno¶æ IPC (wszystko z obustronnym potwierdzeniem
i obustronnym zg³oszeniem gotowo¶ci odbioru / nadawania danych:

- Rejestracja w systemie (ustawienie IPCREG):

  Syscalle: IPC_REGISTER

- zestawianie po³±czeñ strumieniowych (odpowiednik unnamed pipes)
  [request nawi±zania sesji mo¿e byæ wys³any do jednego procesu,
  do wszystkich procesów o danym IPCREG, do wszystkich procesów na danym
  ho¶cie albo do wszystkich hostów w ogóle - broadcast; te mog±, ale
  nie musz± go odebraæ, wywo³uj±c odpowiedni syscall; warto zrobiæ
  niewielk± kolejkê requestów IPC]. Po IPC_CHECK_QUEUE a przed IPC_REJECT
  lub IPC_ACCEPT ostatnia pozycja kolejki powinna byæ "zamra¿ana",

  Syscalle: IPC_STREAM_ASK, IPC_CHECK_QUEUE, IPC_REJECT, IPC_ACCEPT,
            IPC_SEND_DATA, IPC_RECEIVE_DATA, IPC_CLOSE, IPC_STATUS

- zestawianie po³±czeñ blokowych (odpowiednik wspó³dzielenia pamiêci).
  [j.w.]

  Syscalle: IPC_BLOCK_ASK [czê¶æ j.w.] IPC_WRITE_BLOCK, IPC_RECEIVE_BLOCK

- wysy³anie komunikatów (2 x dword);
  [j.w.]

  Syscalle: IPC_SEND_MSG

Co z rIPC? Pomys³ jest równocze¶nie prosty i bardzo przebieg³y. Otó¿
w³a¶ciwie nic. Modu³ rIPC rozszerza funkcjonalno¶æ modu³u IPC pozwalaj±c
linkowaæ ze sob± predefionowane serwery. Struktura linkowania mo¿e byæ
dowolna, ka¿dy serwer posiada odpowiedni identyfikator w hierarchii (numery 
te nie s± u¿ywane do celów innych ni¿ unikanie pêtli). Ka¿dy do³±czony
serwer informuje inne serwery, jakie procesy posiadaj± jakie numery IPCREG,
i zapewnia przesy³anie zapytañ IPCREG w obrêbie po³±czeñ (requesty s±
po nadaniu identyfikatora broadcastowane).

Co z tego wynika?

- mo¿liwo¶æ "rozproszenia" aplikacji miêdzy wiele maszyn bez konieczno¶ci
  wprowadzania jakichkolwiek zmian w kodzie,

- mo¿liwo¶æ uruchomienia w kilku punktach sieci procesów posiadaj±cych
  ten sam identyfikator IPCREG; zapytania do nich bêd± wtedy rozdzielane
  (load balancing / przetwarzanie rozproszone) - round robin

- mo¿liwo¶æ tworzenia klas funkcjonalnie to¿samych maszyn w ró¿nych
  punktach sieci, które s± w stanie realizowaæ zadania równocze¶nie,

- mo¿liwo¶æ tworzenia redundantnej struktury po³±czeñ,

Jak ju¿ mówi³em, wynikaj± z tego niespotykane mo¿liwo¶ci tworzenia
samoorganizuj±cych, adaptuj±cych siê do sytuacji, potrzeb i wymagañ w
sposób nie wymagaj±cy interwencji u¿ytkownika systemów clustrowych. Argante
mo¿e stanowiæ platformê zarz±dzaj±co-kontroln± takich clustrów. Podobnych
mo¿liwo¶ci tworzenia clustrów nie oferuj± chyba ¿adne inne rozwi±zania.

Aha, dystrybucjê requestów miêdzy to¿same procesy powinien wspieraæ te¿
"czysty" IPC ;>


-----------------------------------
10. Skrypty i zarz±dzanie konsolowe
-----------------------------------

Ilekroæ piszê "skrypty nadzorcze", "operator", nie mam na my¶li specjalnego,
uprzywilejowanego konta administratora, tylko obs³ugiwan± z poziomu
kernel-space konsolê zarz±dzaj±c±. W chwili startu systemu, wykonywane s±
skrypty startowe (które mog± miêdzy innymi wczytywaæ modu³y i nawi±zywaæ
po³±czenia sesji rIPC, a tak¿e wczytywaæ procesy).

Zarz±dzanie prac± systemu wirtualnego nie odbywa siê z poziomu zadañ
wykonywanych wewn±trz systemu.

Konsola Argante oferuje do¶æ prosty zestaw poleceñ, które s³u¿± g³ównie do
uruchamiania procesów i zarz±dzania bibliotekami. Oto one:

?               - pomoc

!               - statystyka systemowa

$fn             - wczytanie obrazu binarnego z pliku fn i uruchomienie go
                  na pierwszym wolnym VCPU

%fn		- jak wy¿ej, wczytuje zadanie w trybie RESPAWN (bêdzie ono
		  uruchomione ponownie je¶li dojdzie do zakoñczenia pracy
	          danego procesu poleceniem innym ni¿ HALT)

                  UWAGA: ten tryb s³u¿y do wykonywania programów, które
	          powinny dzia³aæ ca³y czas; generalnie jednak, nale¿y
		  stawiaæ nacisk na prawid³owe funkcjonowanie procesu
		  i obs³ugê wyj±tków w ka¿dej sytuacji, a tak¿e na tworzenie
		  redundantnych procesów w hierarchii IPC, a tê opcjê traktowaæ
	          tylko jako rozwi±zanie pomocnicze. Jako zabezpieczenie
	          przed nadu¿yciem tego trybu, istnieje zmienna
		  MIN_CYCLES_TO_RESPAWN definiowana w pliku config.h, która
		  okre¶la minimaln± ilo¶æ cykli pracy przed wyst±pieniem
		  sytuacji prowadz±cej do ¶mierci procesu (32). Je¶li b³±d
		  wyst±pi wcze¶niej, program nie zostanie uruchomiony ponownie.

>fn             - wczytanie biblioteki z pliku fn do wolnego slotu

<id             - skasowanie biblioteki w slocie 'id'

#               - wy¶wietlenie listy bibliotek wraz ze statystyk±
                  (obs³ugiwane syscalle, ilo¶æ wywo³añ)

@fn             - uruchomienie skryptu konsolowego

-nn             - zabicie procesu na VCPU numer nn

=nn             - wy¶wietlenie statystyki dla procesu na VCPU numer nn
.               - halt systemu

*nn             - wykonanie nn taktów systemu bez sprawdzania wej¶cia
                  na konsoli zarz±dzaj±cej; przydatne w skryptach.

:xx             - wywo³anie subshella i wykonanie polecenia "xx"

|xx             - "nic" - komentarz w skrypcie.

^               - ponowne wczytanie tablicy HAC

w nn tmout      - czekaj na zakoñczenie procesu nn przez tmout sekund

Dostêpne s± tak¿e inne polecenia, s³u¿±ce do debugowania, opisane w
dalszych czê¶ciach dokumentu.

Jak zapewne zauwa¿y³e¶, konsola jest elementem sk³adowym systemu - w tym
sensie, ¿e zarz±dzanie mo¿e odbywaæ siê bezpo¶rednio po zbootowaniu.
Oczywi¶cie jest to tylko wygodna opcja, dostêp do sesji Argante mo¿e byæ
realizowany w inny sposób (patrz polecenia agtback i agtses). W obecnej
chwili nie uwa¿ali¶my za istotne oddzielanie kodu konsoli od samego systemu,
jako ¿e nie jest to w ¿adnym wypadku "kosztowne" rozwi±zanie (nie obni¿a
wydajno¶ci), a zapewnia ³atwo¶æ zarz±dzania w ka¿dej sytuacji. Byæ mo¿e 
system i konsola zostan± rozdzielone w przysz³o¶ci.

Skrypty maj± sk³adniê analogiczn± do poleceñ konsolowych. Po uruchomieniu
systemu wykonywany jest skrypt argboot.scr, lub inny skrypt, je¶li
podany zostanie w linii poleceñ (je¶li podany by³ tak¿e drugi parametr
z linii poleceñ, ten w³a¶nie katalog zostanie przyjêty jako pocz±tkowy
katalog wykonywania - z plikami konfiguracyjnymi, filesystemem etc - jak
d³ugo plik config.h nie definiuje ¶cie¿ek absolutnych, lecz relatywne).
Przyk³ad skryptu:

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

Konsol± domy¶ln± jest stdin procesu 'argante' w chwili odpalenia. Oczywi¶cie,
mo¿e nie byæ to dzia³anie oczekiwane przez administratora. Mo¿liwe jest
wiêc uruchomienie argante w tle w nastêpuj±cy sposób:

tools/agtback sciezka-do-argante [ nazwa-skryptu ]

Nale¿y zwróciæ uwagê na to, by w obecnym katalogu istnia³o odpowiednie
¶rodowisko uruchomieniowe - modu³y, skrypty startowe w odpowiednich
katalogach.

Praca na konsoli sesji pozostawionej w tle jest mo¿liwa z wykorzystaniem
polecenia agtses. Jako parametr nale¿y podaæ mu numer procesu Argante.


-------------------------------------
11. Pos³ugiwanie siê RSIS-assemblerem
-------------------------------------

Tak wygl±da przyk³adowy plik ¼ród³owy, który wy¶wietla liczby od 10 do 0
i przy okazji trochê gada:

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

Ró¿ne przyk³ady (pliki *.agt) mo¿na znale¼æ w podkatalogu compiler/examples -
oprócz podobnego "hello world" jest tam tak¿e np. przyk³ad obs³ugi
wyj±tków (error.agt) czy systemu plików (fs.agt). Sam jêzyk AGT ma nastêpuj±c±
sk³adniê:

.DATA, .CODE - definicje kolejnych segmentow (.data jest opcjonalne).
	       Dziêki bulbie, mo¿esz prze³±czaæ siê miêdzy segmentami
	       kiedy chcesz :)

.END         - koñczy segment kodu

:xxx         - zarówno w segmencie kodu, jak i w segmencie danych, oznacza
               symboliczn± nazwê s³u¿±c± do wskazywania obiektu znajduj±cego
               siê w nastêpnej linii; musi wystêpowaæ w oddzielnej linii,
               w segmencie danych wszystkie obiekty musz± mieæ nazwê.

               Dane mo¿na umieszczaæ w formacie:

                "xxxx" - ci±g znaków
                123    - warto¶æ liczbowa ca³kowita (32-bitowa)
                123.0  - warto¶æ liczbowa zmiennoprzecinkowa (j.w.)
                0x123  - warto¶æ hexadecymalna 32bit

                NN repeat 123 - blok 123 powtórzeñ warto¶ci NN
                                (zmiennoprzecinkowa lub ca³kowita)

                block 100 - nastêpne 100 linii bêdzie zawiera³o warto¶ci
                            (dwords) do wpisania do struktury.

                Wywo³ania do symboli przekazywanych jako parametry, musz±
                mieæ nastêpuj±c± postaæ: ':Symbol'. Inne mo¿liwo¶ci to
                '^Symbol' - zwraca d³ugo¶æ obiektu w bajtach, przydatne dla
                ci±gów tekstowych; '%Symbol' - zwraca d³ugo¶æ w dwords.

!xxx         - dyrektywa kompilacji, okre¶la parametry procesu. Dopuszczalne:

               !DOMAINS x x x     - lista grup wykonywania
               !PRIORITY x        - priorytet programu
               !IPCREG x          - pocz±tkowy identyfikator IPC
               !INITDOMAIN x      - pocz±tkowa grupa wykonywania
               !SIGNATURE x       - sygnatura kodu (opis, autor)
               !INITUID x	  - pocz±tkowy identyfikator podgrupy

Dopuszczalne jest identyfikowanie syscalli na podstawie ich symbolu, o ile
ten jest znany kompilatorowi. Lista syscalli znajduje sie w syscall.h w
katalogu modules/. Nazwa syscalla musi byc poprzedzona znakiem $ - na
przyk³ad 'syscall $io_putstring' (uwaga - pomijamy przedrostek syscall_).
Tak samo mo¿na odwo³ywaæ siê do numerów wyj±tków - ich nazwy znajduj± siê
w pliku include/exception.h, tu pomijamy error_.

Aha - priorytet '1' jest domy¶ln± warto¶ci±, aczkolwiek nie jest zbyt
rozs±dny. Sugerujê priorytety w granicach 10-10000, poniewa¿ wtedy w ka¿dym
cyklu wykonywane jest wiêcej poleceñ maszynowych, a obróbka ich za jednym
zamachem jest efektywniejsza ni¿ kolejne skoki.

Rejestry nale¿y podawaæ w formacie 'xNN', gdzie NN to numer rejestru,
a x to odpowiednio 'u' (ureg), 's' (sreg), 'f' (freg). I tak na przyk³ad 'u0'
to ureg[0].

Poprzedzenie warto¶ci liczbowej, symbolu lub rejestru 'u' znakiem '*'
oznacza, ¿e chodzi o warto¶æ znajduj±c± siê pod danym adresem. Na przyk³ad:

mov u0,*:Test

Spowoduje wpisanie do rejestru u0 warto¶ci znajduj±cej siê pod adresem Test,
podczas gdy:

mov u0,:Test

Spowoduje wpisanie adresu wskazywanego przez identyfikator 'Test' do rejestru
u0.

Kompilator, przynajmniej w obecnej wersji, nie bêdzie wspiera³ arytmetyki
na poziomie kompilacji. System nie wspiera tak¿e wielu rozdzielnych
bloków pamiêci przydzielanych w chwili ³adowania binarki.

Kompilator uruchamia siê wpisuj±c "compiler/agtc plik.agt". Efektem bêdzie
plik binarny plik.img, który mo¿na za³adowaæ poleceniem $ w konsoli
zarz±dzaj±cej.


--------------------------------------------------------
12. Pos³ugiwanie siê translatorem AHLL <w przygotowaniu>
--------------------------------------------------------

0) Wprowadzenie do AHLL
~~~~~~~~~~~~~~~~~~~~~~~

Generalnie, konwencje AHLL wahaj± siê miêdzy C (w kwestiach sk³adni)
do Ady (filozofia wska¼ników, w³a¶ciwo¶ci, kontroli sk³adni). Oczywi¶cie
si³± rzeczy ten jêzyk jest tylko "marnym" podzbiorem jêzyków macierzystych,
aczkolwiek w pe³ni wystarcza do pisania nawet ca³kiem z³o¿onych projektów
- przynajmniej tak± mam nadziejê i takie s± za³o¿enia.

Je¶li znasz Pascala i C, a tak¿e masz pojêcie o Argante, powiniene¶ nie mieæ
¿adnych problemów ze zrozumieniem filozofii AHLL.

Oczywi¶cie jêzyk ten powinien byæ du¿o bardziej urozmaicony - ale tak siê
sk³ada, ¿e system ten powstaje od podstaw, i muszê stawiaæ sobie w miarê
realne cele, które stopniowo realizujê. My¶lê, ¿e ten jêzyk i jego specyfikacja
powinna byæ dalej rozwijana, ja po prostu stwarzam podstawy.


1) Definicje globalne
~~~~~~~~~~~~~~~~~~~~~

Defniowanie zmiennych wygl±da generalnie w nastêpuj±cy sposób:

nazwa_zmiennej	:  nazwa_typu [ (parametry) ]  [ := Wartosc_Poczatkowa ] ;

W przypadku typów z³o¿onych (tablic lub struktur) wartosc_poczatkowa mo¿e
byæ zbiorem warto¶ci ujêtym w nawiasy klamrowe, na przyk³ad:

    moja_zmienna : moj_typ_tablicowy := { 0, 1, 2, 3 };

Nazwa zmiennej, jak i wszystkie inne symbole AHLL, jest case-insensitive,
i podlega tradycyjnym konwencjom - mo¿e sk³adaæ siê ze znaków alfanumerycznych
oraz podkre¶lenia ("_"), musi zaczynaæ siê od litery. Maksymalna d³ugo¶æ
identyfikatora to 64 znaki.

Nazwa typu podlega takim samym zasadom. Typ ten musi byæ typem
predefiniowanym lub jednym ze zadeklarowanych wcze¶niej typów. Je¶li
podany jest inicjalizator, musi byæ on warto¶ci± daj±c± siê okre¶liæ w
chwili kompilacji, i musi posiadaæ ten sam typ oraz rozmiar, co inicjalizowany
obiekt.

Niektóre typy pozwalaj± podawaæ parametry w chwili inicjalizacji. Parametry
te mog± okre¶laæ pocz±tkowe i/lub koñcowe brzegi przedzia³u warto¶ci danego
typu lub rozmiar tablicy. Wywo³anie wygl±da wtedy nastêpuj±co:

  moja_zmienna : moj_typ_tablicowy(100);

Je¶li konieczne jest, by jako parametr przy inicjalizacji typu przekazano
warto¶æ okre¶laj±c± w jaki¶ sposób inicjalizator, mo¿e odbyæ siê to przez
skorzystanie ze specjalnego symbolu "Self" posiadaj±cego nastêpuj±ce 
"w³asno¶ci" dla typów prostych:

Self'Dword_Length	- okre¶la d³ugo¶æ inicjalizatora w dwords
Self'Byte_Length	- okre¶la d³ugo¶æ inicjalizatora w bajtach
Self			- warto¶æ inicjalizatora.

Oraz nastêpuj±ce dla tablic / typów strukturalnych:

Self'Count		- ilo¶æ elementów inicjalizatora
Self[n]'Dword_Length    - d³ugo¶æ n-tego pola inicjalizatora w dwords
Self[n]'Byte_Length     - d³ugo¶æ n-tego pola inicjalizatora w bajtach
Self[n]			- warto¶æ n-tego pola inicjalizatora

Uwaga: mo¿liwe jest tylko bezpo¶rednie inicjalizowanie elementów tablicy
lub struktury, bez "zazêbiania" inicjalizacji. A wiêc poprawn± konstrukcj±
jest:

 [ ...definicja typu Ala, ktory jest struktura zawierajaca m ciagi znakow... ]
 [ ...definicja typu Kot, ktory jest tablica n obiektow typu Ala... ]

 ala_1   : Ala (Self'Count) := { "kot", "ma", "Ale" };
 ala_2   : Ala (Self'Count) := { "Ala", "ma", "kota" };
 moj_kot : Kot (Self'Count) := { ala_1, ala_2, ala_1 };

Je¶li jako czê¶æ inicjalizatora trzeba przekazaæ warto¶æ okre¶laj±c± w jaki¶
sposób inny jego element, mo¿na zrobiæ to w nastêpuj±cy sposób:

  ala : BoundedChunk := { "tekst", Self[1].Byte_Length };
 
Natomiast nieprawid³owe by³oby rozwi±zanie d±¿±ce do po³±czenia inicjalizacji
tablicy (Kot) z inicjalizacj± struktury (Ala):

  // Blad!
  moj_kot : Kot (Ini'Count) := { { "kot", "ma", "Ale" } , { "Ala", ... } ... };

Dlaczego? No có¿, choæby w celu unikniêcia problemów z to¿samo¶ci± elementów,
z przekazywaniem ewentualnych parametrów do inicjalizatorów struktur w
tablicy etc.

Predefiniowane typy to:

  unsigned	- unsigned integer, 4 bajty

  signed	- signed integer, 4 bajty

  float		- float, 4 bajty

  proc_address	- specjalny typ s³u¿±cy do przechowywania adresu procedur;
                  typ ten nie mo¿e byæ ograniczony z wykorzystaniem
                  modyfikatora range

Semantyka AHLL zak³ada, i¿ nie jest mo¿liwe przeprowadzanie przypisañ miêdzy
ró¿nymi typami. Dopuszczalne jest przeprowadzanie konwersji za pomoc± dyrektywy
Convert(zmienna) - pod warunkiem, i¿ typy s± typami prostymi.

Nale¿y liczyæ siê z tym, ¿e je¶li warto¶æ konwertowana wykracza poza granice
warto¶ci dopuszczalnych dla warto¶ci docelowej, dojdzie do zg³oszenia
wyj±tku (patrz sekcja 5).

Definiuj±c zmienne, mo¿na pos³u¿yæ siê nastêpuj±cymi modyfikatorami przed
nazw± typu:

  pointer to	- warto¶æ jest wska¼nikiem. Nie mo¿e byæ u¿ywana do czasu,
                  gdy nie zostanie zaalokowana (lub nie zacznie wskazywaæ
                  na inny obiekt danego typu) ani po tym, jak zostanie
                  zwolniona.

  addressable   - obiekt mo¿e byæ wskazywany przez warto¶ci wska¼nikowe,
                  program mo¿e mieæ dostêp do atrybutu 'Address.

Wykorzystywanie tych typów w celu uzyskania tablic, struktur itp, zosta³o
opisane w sekcji 3.


2) Prekompilacja
~~~~~~~~~~~~~~~~

Sta³e mo¿na wprowadziæ do programu korzystaj±c z opcji prekompilatora:

#define nazwa_stalej wartosc

Mo¿liwe jest tak¿e w³±czanie kodu z innych plików do zasadniczego kodu,
u¿ywaj±c dyrektywy:

#include "nazwa_pliku"

Dopuszczalne komenatrze (bez nestingu) s± w konwencji C: // lub /* */

Kompilator wspiera tak¿e przekazywanie kodu dla agtc - przez
dyrektywê #compiler. Na przyk³ad:

#compiler !DOMAINS 1 2

Na razie kompilator nie obs³uguje ¿adnej innej funkcjonalno¶ci.


3) Definiowanie nowych typów
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Istniej± dwa rodzaje typów: typy w³a¶ciwe, oraz podtypy. O ile typy
w³a¶ciwe nie daj± siê przepisywaæ miêdzy sob± (np. nie mo¿na przypisaæ
bezpo¶rednio warto¶ci zmiennej typu Typ1 do zmiennej typu Typ2, nawet
je¶li za oboma typami kryje siê tylko typ unsigned), o tyle podtypy
mog± byæ przpisywane miêdzy sob±. A wiêc Podtyp1 i Podtyp2, o ile s±
podtypami tego samego typu, daj± siê miêdzy sob± przepisywaæ.

Aby zdefiniowaæ typ, u¿ywa siê nastêpuj±cej dyrektywy:

[sub]type nazwa_typu is typ_bazowy [ range aa .. bb ];

type nazwa_typu is array aa .. bb of typ_bazowy;

type nazwa_typu is bytechunk aa .. bb;

type nazwa_typu is structure {
  obiekt : [modyfikatory] typ;
  ...
}

U¿ycie modyfikatora "range" mo¿liwe jest dla typów prostych (i tylko dla 
nich). Dziêki jego wykorzystaniu mo¿na ograniczyæ zakres warto¶ci, które
mog± byæ przyjmowane przez dan± zmienn±.

Modyfikator array powoduje utworzenie tablicy obiektów typu typ_bazowy,
który mo¿e byæ albo typem predefiniowanym, albo typem zdefiniowanym przez
u¿ytkownika. Nie jest mo¿liwe tworzenie podtypów tablicowych, poniewa¿
nie mia³oby to wiêkszego sensu. W przypadku tablicy, jest mo¿liwa 
deklaracja umo¿liwiaj±ca podanie jej rozmiaru w chwili definiowania
zmiennej:

type nazwa_typu is variable array of typ_bazowy;

Wtedy definicja zmiennej musi wygl±daæ np. nastêpuj±co:

zmienna : nazwa_typu(1,10);

Mo¿liwa jest tak¿e deklaracja typu z pominiêciem jednego lub drugiego
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

Modyfikator structure { ... } pozwala zdefiniowaæ strukturê, czyli blok
sk³adaj±cy siê z obiektów sk³adowych, wymienionych w segmencie { ... }.
W sk³ad struktury mog± wchodziæ dowolne obiekty o dowolnym typie lub
wska¼niki do tych obiektów, o ile zosta³y zdefiniowane wcze¶niej.

W celu jednoznaczno¶ci deklaracji nie jest mo¿liwe definiowanie typów
bazuj±c na anonimowych typach bazowych. Nie jest wiêc poprawn± konstrukcja:

type nazwa_typu is array aa .. bb of structure {
  ...
};

Poniewa¿ nie jest okre¶lona nazwa struktury. Deklaracjê tê nale¿y roz³o¿yæ
na dwie oddzielne, pierw okre¶laj±c nazwê struktury.

Ka¿da zmienna posiada pewne w³a¶ciwo¶ci, dostêpne w trybie read-only. S± to:

Zmienna'First		- dolna granica range lub dolna warto¶æ tablicy
Zmienna'Last            - jw, górna
Zmienna'Count           - 'Last - 'First
Zmienna'Byte_Length     - rozmiar obiektu w bajtach
Zmienna'Dword_Length    - rozmiar obiektu w dwords
Zmienna'Address		- adres zmiennej (dostêpny tylko je¶li zmienna
                          zosta³a zdefiniowana z parametrem addressable lub
                          jest wska¼nikiem)

Warto¶ci inicjalizuj±ce dla bytechunk'a mog± byæ list± warto¶ci liczbowych
tak jak w przypadku tablicy ({1,2,3}) lub tekstem ujêtym w cudzys³owy
("tekst").

W obecnej chwili struktury nie beda wspieraly inicjalizatorow pol oraz
korzystania z typow, ktore wymagaja inicjalizacji - zmieni sie to 
prawdopodobnie w nastepnej wersji AHLL.


4) Wska¼niki
~~~~~~~~~~~~

Wska¼niki wystêpuj± w formie niejawnej, podobnie jak w Adzie. Jest wiêc
mo¿liwe stworzenie zmiennej z atrybutem "pointer to", ale od chwili
przypisania jej warto¶ci do jej wymazania, obiekt ten podlega dok³adnie
takim samym prawom, jak zwyk³y obiekt danego typu. Mo¿liwe s± nastêpuj±ce
operacje:

Create(zmienna);	- stworzenie nowego obiektu i przypisanie go "pod"
			  wska¼nik "zmienna"; operacja powiedzie siê tylko
		          je¶li zmienna jest obiektem wska¼nikowym i nie
                          jest do niej w danej chwili zbindowana warto¶æ.

Unbind(zmienna);        - od³±czenie (odbindowanie) wska¼nika od zmiennej;
                          normalnie, gdy zostanie opuszczona funkcja, w
                          której zaalokowana zosta³a warto¶æ z wykorzystaniem
                          Create(); dopiero po tej operacji mo¿liwe jest
                          ponowne wywo³anie Create() na danej zmiennej.

Bind(zmienna,nowa);	- przy³±czenie (zbindowanie) wska¼nika do istniej±cej
			  zmiennej 'nowa'; mo¿liwe tylko je¶li zmienna 'nowa'
			  by³a zdefiniowana z atrybutem addressable.
                          Jest to rownowazne z konstrukcja zmienna := nowa;

Destroy(zmienna);	- zniszczenie obiektu (tylko je¶li by³ stworzony
                          funkcj± Create).


Dla wygody, zmienne wska¼nikowe udostêpniaj± dwa dodatkowe atrybuty:

Zmienna'Binded		- czy wska¼nik jest zbindowany?
Zmienna'Used		- czy jest przypisana warto¶æ?


4) Range-checking i wyj±tki
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Range checking przeprowadzany jest w nastêpuj±cych sytuacjach:

- jawna konwersja typu funkcji
- niejawna konwersja (przypisanie)
- dostêp do tablicy
- dynamiczne tworzenie obiektu w bloku local { ... }

Kompilator generuje kod zg³aszaj±cy wyj±tki w nastêpuj±cych sytuacjach:

- B³êdy range checkingu
- Create() / Bind() na zbindowanym wska¼niku
- Destroy() na niezbindowanym wska¼niku
- Unbind() na niezbindowanym wska¼niku
- ...plus wyj±tki systemowe

Przy innych b³êdach (typu Create() na obiekcie nie bêd±cym wska¼nikiem,
Bind() do obiektu który nie jest addressable) powinny byæ zg³aszane b³êdy
kompilacji, co jest proste.


5) Procedury
~~~~~~~~~~~~

W AHLL nie ma funkcji, tylko procedury. Procedura wygl±da nastêpuj±co:

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

Pierw deklarowane s± parametry, wraz z opisuj±cymi je nazwami typów. Potem,
nastêpuje blok local { ... }, gdzie mog± znajdowaæ siê deklaracje lokalnych
zmiennych. Potem znajduje siê normalny kod - sk³adaj±cy siê z przypisañ,
konstrukcji warunkowych, pêtli oraz wywo³añ innych procedur. Koñcowy blok
exception { ... }, omówiony w sekcji 7, odpowiada za obs³ugê wyj±tków :>
Blok protected { ... } jest kodem, w obrêbie którego maj± byæ obs³ugiwane
wyj±tki (mo¿e wystêpowaæ dowolnie czêsto).

Modyfikator addressable przed nazw± procedury pozwala na pobranie jej
adresu (typu proc_addres) przez udostêpnienie w³asno¶ci 'IP_Address


6) Syscalle i standardowe funkcje
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Jako interfejs do kernela, istnieje specjalna dyrektywa syscall o
nastêpuj±cej postaci:

  syscall(numer, u0 := nn, u1 := nn, ..., nn := u0 ...);

Numer syscalla jest oczywi¶cie systemowym numerem wywo³ania. Nastêpnie
wystêpuje blok przypisañ do specjalnych zmiennych o nazwach uX, sX lub
fX, czyli rejestrów VCPU, oraz blok przypisañ Z rejestrów ( nn = uX, sX, fX).
Pierwszy blok oznacza parametry dla wywo³ania syscalla, drugi - warto¶ci
powrotne. A wiêc na przyk³ad:

// To powinno znale¼æ siê w pliku nag³ówkowym do obs³ugi ³añcuchów tekstowych.

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


7) Obs³uga wyj±tków
~~~~~~~~~~~~~~~~~~~

S³u¿y do nich blok exception { }, opcjonalny na koñcu ka¿dej procedury.
Zostanie on wykonany w przypadku zg³oszenia wyj±tku. Jego budowa jest
podobna jak dla switch:

exception {
  wartosc: kod [...]
  wartosc: kod [...]
  [...]
  [ default: kod... ]
}

Zg³aszanie wyj±tków mo¿e siê odbywaæ z wykorzystaniem predefinowanej
dyrektywy raise(numer_wyj±tku). Nieobs³u¿ony wyj±tek prowadzi do zakoñczenia
pracy programu.

Zakoñczenie pracy programu jest mo¿liwe tak¿e za pomoc± dyrektywy halt().

Mo¿e to nienajlepsze miejsce, ale zawsze. Inne predefiniowane dyrektywy
dotycz±ce wykonywania programu:

  cycle_wait(nn) - wprowadzenie zadania w stan CWAIT
  time_wait(nn)  - wprowadzenie zadania w stan TWAIT


8) Arytmetyka i odwo³ania do tablic/struktur
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Odwo³ania do elementów tablic: tablica[numer_elementu]
Odwo³ania do elementów struktury: struktura.nazwa_elementu
Odwo³ania mog± byæ kombinowane - np. struktura.nazwa_tablicy[numer_elementu].

Przypisania:

  zmienna := wartosc;

Operatory arytmetyczne (w miarê mo¿liwo¶ci powinny byæ liczone w trakcie
kompilacji): +, -, *, / - standardowe operacje.

Operatory binarne: AND, OR, XOR, NOT.

Operatory logiczne porównañ: <>, =, <, >, >=, <=.

Kolejno¶æ obliczeñ: ().

Przyk³ad:

  zmienna := ( 2 > 1 ); // Przypisana bêdzie warto¶c 1 ;)

Nie ma oddzielnego typu logicznego.


9) Bloki warunkowe, kontrola wykonywania
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Wywo³anie procedury: nazwa_procedury(parametr,parametr...);
Deklaracja labela: :nazwa_labela
Skok do labela (musi byc w tej samej funkcji): goto nazwa_labela

Zakoñczenie programu: terminate();

Wykonywanie warunkowe:

  if (wyrazenie_niezerowe) {
    ...
  } [ elif (wyrazenie niezerowe | else { ... } 

Pêtle:

  do while (wyrazenie_niezerowe) {
    ...
  }

  do {
    ...
  } while (wyrazenie_niezerowe);

Kontrola pêtli: continue, break - jak w C

Wyj¶cie z procedury: return

W bloku exception {} dopuszczalne s±:

  return   - powrot z procedury
  ignore   - kontynuowanie w punkcie, gdzie wyst±pi³ wyj±tek

  terminate(), rzecz jasna
  

Bloki switch:

  switch (wartosc) {
    mozliwosc: ...kod...
    mozliwosc: ...kod...
    default: ...kod...
  }

Default case musi istnieæ, mo¿e byæ pusty.

10) Przyk³adowy program
~~~~~~~~~~~~~~~~~~~~~~~

-- hello.ahl --
#include "stdinc.ahh"
#include "display.ahh"

procedure Main() {
  Console_PutString( { "Hello world.\n", Self[1]'Byte_Length } );
}
-- EOF --


--------------------------------------
13. Specyfikacja modu³ów standardowych
--------------------------------------


  Modu³ display.c
  ---------------

  Status: zamro¿ony

  Przeznaczenie: wy¶wietlanie podstawowych danych na konsoli z poziomu
  procesu u¿ytkownika; debugging etc. UWAGA: modu³ nie powinien byæ
  u¿ywany do interakcji z u¿ytkownikiem. Powstanie do tego inne rozwi±zanie,
  sugerowanym obecnie sposobem jest modu³ obs³ugi sieci.

  Syscall: IO_PUTSTRING

            Parametry: u0 - adres ci±gu znaków
                       u1 - ilo¶æ znaków

            Efekt: wy¶wietla ci±g znaków

            Wyj±tki: BAD_PROTFAULT - próba wy¶wietlenia b³êdnego fragmentu
                     pamiêci

            HAC: operacja=display/output/text obiekt=none

  Syscall: IO_PUTINT

            Parametry: u0 - warto¶æ do wy¶wietlenia

            Efekt: wy¶wietla warto¶c liczbow±

            Wyj±tki: -

            HAC: operacja=display/output/integer obiekt=none

  Syscall: IO_PUTCHAR

            Parametry: u0 (najm³odsze 8 bitów) - znak do wy¶wietlenia

            Efekt: wy¶wietla znak ascii

            Wyj±tki: -

            HAC: operacja=display/output/character obiekt=none


  Modu³ access.c
  --------------

  Status: zamro¿ony

  Przeznaczenie: zarz±dzanie przywilejami; zmiana aktywnej domeny (grupy)
  i identyfikatora podgrupy. Wsparcie systemu HAC.

  Kontrola dostêpu: brak.

  Syscall: ACCESS_SETDOMAIN

            Parametry: u0 - numer grupy

            Efekt: zmienia aktywn± grupê

            Warunek: grupa nale¿y do zbioru !domains okre¶lonego w czasie
                     kompilacji

            Wyj±tki: NOPERM - je¶li grupa nie znajduje siê w w/w
                               zbiorze.

            HAC: brak wsparcia

  Syscall: ACCESS_SETUID

            Parametry: u0 - numer identyfikatora podgrupy

            Efekt: zmienia aktywn± podgrupê

            Warunek: brak

            Wyj±tki: brak

            HAC: brak wsparcia


  Modu³ fs.c
  ----------

  Status: wdra¿anie

  Przeznaczenie: dostêp do SVFS.

  Kontrola dostêpu: HAC + istnienie obiektów w hierarchi SVFS.
  Wyj±tki: standardowe + FSERROR - b³±d dostêpu do zasobów SVFS.

  Syscall: FS_OPEN_FILE

            Parametry: u0 - adres nazwy pliku, u1 - d³ugo¶c nazwy pliku
                       u2 - flagi: FS_FLAG_READ, FS_FLAG_WRITE,
                                   FS_FLAG_APPEND, FS_FLAG_NONBLOCK
 
            Efekt: otwiera podany plik w odpowiednim trybie
                   s0 - VFD (virtual file descriptor); -1 = plik zalockowany

            Uwagi: je¶li nie jest podana flaga NONBLOCK, a zg³oszono
                   próbê otwarcia pliku do zapisu i jednocze¶nie inny
		   proces zapisuje dane do pliku, proces wchodzi w stan
                   IOWAIT do czasu uzyskania dostêpu; NONBLOCK powoduje
		   natychmiastowe zwrócenie warto¶ci -1.

            HAC: fs/fops/open/file/{read|write|append}

  Syscall: FS_CREATE_FILE

            Parametry: u0 - adres nazwy pliku, u1 - d³ugo¶c nazwy pliku
                       u2 - flagi: FS_FLAG_WRITE, FS_FLAG_APPEND
 
            Efekt: tworzy podany plik w odpowiednim trybie
                   zwraca s0 - VFD (virtual file descriptor)

            HAC: fs/fops/create/file/{write|append}


  Syscall: FS_CLOSE_FILE

            Parametry: u0 - numer VFD
 
            Efekt: zamkniêcie VFD, koniec pracy z plikiem

            HAC: brak


  Syscall: FS_WRITE_FILE

            Parametry: u0 - numer VFD, u1 - wska¼nik, u2 - d³ugo¶æ (bajty!)
 
            Efekt: zapisanie danych do pliku, je¶li pozwalaj± na to
                   uprawnienia dostêpu do VFD i pamiêci

            HAC: brak


  Syscall: FS_READ_FILE

            Parametry: u0 - numer VFD, u1 - wska¼nik, u2 - d³ugo¶æ (bajty!)
 
            Efekt: odczyt danych z pliku do pamiêci, je¶li pozwalaj± na to
                   uprawnienia dostêpu do VFD i pamiêci

            HAC: brak


  Syscall: FS_SEEK_FILE

            Parametry: u0 - numer VFD, u1 - pozycja, u2 - typ
 
            Efekt: sk³adania analogiczna do lseek() w libc. Na plikach
		   dostêpnych w trybie append() tylko u1=0, u2=1 (current)
                   jest akceptowane (zwraca obecn± pozycjê).

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
                   do s0 faktycznej d³ugo¶ci nazwy

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

   [ pozosta³o czytanie katalogu - do zrobienia ]


  Modu³ locallib.c
  ----------------

  Status: wdra¿anie

  Przeznaczenie: dostêp do zasobów systemowych.

  Kontrola dostêpu: HAC
  Wyj±tki: standardowe

  Syscall: LOCAL_GETTIME

            Efekt: u0 - sekundy, u1 - mikrosekundy

            HAC: local/sys/real/time/get

  Syscall: LOCAL_TIMETOSTR

            Parametry: u0 - zwrócone przez GETTIME, u1 - adres bufora,
                       u2 - rozmiar bufora

            Efekt: wpisuje string do bufora, w s0 zwraca ilo¶æ
                   znaków.

            HAC: brak

  Syscall: LOCAL_GETHOSTNAME

            Parametry: u0 - adres bufora, u1 - rozmiar bufora

            Efekt: wpisuje lokaln± nazwê komputera do bufora, s0 - ilo¶æ
                   znaków.

            HAC: local/sys/real/hostname/get

  Syscall: SYSCALL_GETRANDOM

	    Efekt: u0 - losowy dword; funkcja pobiera dword z lokalnego
                        ¼ród³a entropii (/dev/urandom)

            HAC: local/sys/random/get

   Syscall: SYSCALL_LOCAL_VS_STAT

            Efekt: u0 - ilo¶æ aktywnych VCPUs; u1 - ilo¶æ cykli idle od startu,
                   u2 - ilo¶æ cykli pracy od startu, u3 - ilo¶æ syscalli,
                   u4 - ilo¶æ z³ych syscalli, u5 - fatal errors...

            HAC: local/sys/virtual/stat 

   Syscall: SYSCALL_LOCAL_RS_STAT

            Efekt: u0 - uptime w sekundach, u1 - load average (1 min),
                   u2 - ilo¶æ RAM w kB, u3 - wolny RAM w kB, u4 - ilo¶æ
                   swapa w kB, u5 - wolny swap w kB, u6 - ilo¶æ procesów RS.

            HAC: local/sys/real/stat





UWAGA: Ka¿dy modu³ korzystaj±cy z HAC, oprócz standardowych wyj±tków, mo¿e
tak¿e zwróciæ: ACL_PROBLEM, NOPERM.

Szkic pozosta³ych modu³ów:

Obecnie tworzone modu³y:

advmem.c        - zaawansowane operacje na pamiêci (w celu przyspieszenia
                  z³o¿onych zadañ wyszukiwania, porównywania, przepisywania)

                  Kontrola dostêpu: brak

locallib.c	- przydatne funkcje systemowe - odczyt zegara czasu
                  rzeczywistego etc.

		  Kontrola dostêpu: HAC

ipc.c           - komunikacja miêdzyprocesowa - nawi±zywanie sesji, wysy³anie
                  i odbieranie danych, wspó³dzielenie pamiêci.

                  Kontrola dostêpu: HAC

network.c       - dostêp do us³ug sieciowych (tcp, unix sockets)

                  Kontrola dostêpu: HAC

remote_ipc.c    - remote IPC. Modu³ zastêpuj±cy IPC z funkcjami serwer-klient
                  i mo¿liwo¶ci± zestawiania po³±czeñ

                  Kontrola dostêpu: HAC


---------------------
14. Tworzenie modu³ów
---------------------

W obecnym rozwi±zaniu s± to dynamicznie linkowane programy w C lub Adzie.
Wymagania s± nastêpuj±ce:

- istnienie syscall_load(int* x); funkcja ta wywo³ywana jest w chwili
  wczytania modu³u, ma on obowi±zek wype³niæ tablicê warto¶ci x numerami
  syscalli, które ma zamiar obs³ugiwaæ; warto¶ci tych syscalli znajduj± siê
  w pliku syscall.h (oczywi¶cie dla nowych funkcji nale¿y dodaæ nowe i
  umie¶ciæ je w syscall.h). Lista nie mo¿e przekraczaæ MAX_SERVE z pliku
  config.h i musi byæ zakoñczona warto¶ci± ujemn±.

- istnienie syscall_handler(int c,int sysnum) - ta funkcja bêdzie wywo³ana
  je¶li VCPU o numerze 'c' wywo³a syscalla o numerze znajduj±cym siê na
  li¶cie zarejestrowanej dla tego modu³u (konkretny numer podawany jest w
  sysnum). Warto¶æ 'c' pozwala na odwo³ywanie siê do struktury vcpu_struct
  zdeklarowanej w task.h (odsy³am po szczegó³y).

- opcjonalnie, istnienie syscall_unload, wykonywanego przy koñczeniu pracy
  syscalla,

- opcjonalnie, istnienie syscall_task_cleanup, który zostanie wykonany
  zawsze gdy jakie¶ zadanie koñczy pracê (usuniêcie otwartych deskryptorów
  itp),

D³ugo mo¿na omawiaæ budowê modu³ów, wiêc wklejê tu fragment przyk³adowego,
obs³uguj±cego prymitywne wyj¶cie na konsole:

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

Wywo³anie funkcji non_fatal s³u¿y do zg³aszania wyj±tków.

Wymiana bibliotek: za³adowanie nowej do dowolnego wolnego slotu, nastêpnie
od³adowanie starej z jej pierwotnego slotu. Nie nast±pi przerwa w obs³udze
syscalli.

Aha, syscalle NIE mog± blokowaæ pracy systemu - dok³adnie jak w np. Linuxie.
Dlatego je¶li konieczne jest oczekiwanie na jak±¶ operacjê (np. recv()),
zaleca siê ustawienie stanu procesu (cpu[nn].state) dodaj±c flagê
VCPU_STATE_IOWAIT i rownoczesne ustawienie cpu[nn].iohandler tak, by
wskazywa³ na funkcjê akceptuj±c± pojedynczy parametr (numer VCPU):
int handler(int cpu_num);

Dodatkowo dostêpne jest pole cpu[nn].iowait_id, okre¶laj±ce identyfikator
zasobu, na który proces oczekuje.

Od tej chwili, proces nie bêdzie pracowa³ (sytuacja analogiczna do
STATE_SLEEP). Zamiast tego, w ka¿dym cyklu obs³ugi zadañ, bêdzie wywo³ywana
funkcja iohandler(numer_cpu). Funkcja powinna sprawdziæ numer zasobu,
na który zadanie oczekuje. Je¶li dostêp jest niemo¿liwy, powinna zwróciæ
0. Je¶li sta³ siê mo¿liwy, funkcja powinna odpowiednio obs³u¿yæ wyniki,
a nastêpnie zwróciæ warto¶æ niezerow± (np. 1) aby automatycznie opu¶ciæ
stan IOWAIT.

Dany modu³ powinien sam zadbaæ o przechowywanie informacji na temat tego,
gdzie dla danego zadania zapisaæ informacje powrotne, etc.

Do wej¶cia w stan IOWAIT najbezpieczniej u¿yæ makra:

ENTER_IOWAIT(numer_cpu,numer_zasobu,iohandler)

Nale¿y pamiêtaæ, by nie przekazywaæ ani nie pobieraæ od procesu "go³ych"
obiektów rzeczywistego systemu - np. numerów deskryptorów plików, ani
nie powierzaæ systemowi kontroli uprawnieñ (np. próbowaæ pisaæ do
deskryptora, a potem sprawdzaæ czy wysz³o). Argante zapewnia pe³n± kontrolê
po w³asnej stronie, w zunifikowany sposób, za¶ wszystkie "rzeczywiste"
obiekty przechowuje w oddzielnych dla ka¿dego procesora tablicach,
podaj±c procesowi co najwyj¿ej identyfikator w obrêbie tych tablic. Najlepszym
przyk³adem poprawnej konstrukcji modu³ów jest modu³ fs.

Oto krótki opis filozofii obs³ugi stringów na niskim poziomie (co nie
interesuje zapewne programisty AHLL, ale jest istotne przy tworzeniu modu³ów),
który napisa³em dla Artura:

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
15. Format plików wykonywalnych
-------------------------------

Format nag³ówka pliku wykonywalnego jest nastêpuj±cy:

  unsigned int magic1;

    Sta³a sygnatura pliku, warto¶æ 0xdefaced

  char domains[MAX_EXEC_DOMAINS];

    Lista domen, do których przynale¿y program, zakoñczona warto¶ci±
    zerow±.

  unsigned int flags;

    Pocz±tkowe flagi procesu. W obecnej chwili ¿adne flagi nie s±
    supportowane.

  unsigned int priority;

    Priorytet okre¶la, jak d³ugi timeslice jest przyznawany procesowi w
    ka¿dym cyklu obs³ugi. Priority wynosz±ce 1 powoduje, i¿ proces za ka¿dym
    razem mo¿e wykonaæ 1 instrukcjê.

  unsigned int ipc_reg;

    To pocz±tkowy identyfikator IPC. Je¶li jest warto¶ci± dodatni±, zostanie
    przepisany do VCPU.

  unsigned int init_IP;

    Pocz±tkowy instruction pointer, zwykle wystarczy 0.

  int current_domain;
  int domain_uid;

    Aktualna domena wykonywania i UID. Uwzglêdniane tylko gdy s±
    warto¶ciami dodatnimi.

  unsigned int bytesize;

    Rozmiar obrazu kodu.

  unsigned int memflags;

    Flagi pamiêci (READ|WRITE, etc)...

  unsigned int datasize;

    Rozmiar obrazu danych.

  char signature[64];

    Opcjonalna sygnatura autora / krótki opis programu.

  unsigned int magic2;

    Sta³a sygnatura 0xdeadbeef

Nastêpnie nastêpuje blok obrazu kodu (rozmiar - 12*bytesize) oraz blok
danych (opcjonalny, rozmiar 4*datasize). Oba bloki s± mapowane od adresu 0
odpowiednio w przestrzeni kodu i danych.

Do modyfikacji tre¶ci nag³ówków ju¿ istniej±cych programów s³u¿y program
Bulby - dostêpny w tools/binedit.c

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

Oprócz debuggera, z33d napisa³ tak¿e deassembler, znajduj±cy siê w katalogu
tools/. Nie jest jeszcze dokoñczony, tym niemniej swoj± rolê spe³nia dobrze.


-------
17. FAQ
-------

Poni¿ej znajduje siê niewielki zbiór odpowiedzi na najczê¶ciej zadawane 
pytania. Wiêkszo¶æ tych odpowiedzi zawiera informacje znajduj±ce siê ju¿
wy¿ej, ale czasem ³atwo co¶ przeoczyæ - w ka¿dym razie czêsto jeste¶my
pytani w nastêpuj±cy sposób:

1) Po co to wszystko?

Dla przyjemno¶ci. Piszemy Argante nie dlatego, i¿ chcemy stworzyæ drugiego
Linuxa - raczej chcieli¶my sprawdziæ, czy jest mo¿liwe zaprojektowanie
systemu, który ³±czy³by bezpieczeñstwo z funkcjonalno¶ci±, wydajno¶ci±,
uniwersalno¶ci± - a równocze¶nie zrywa³by z wiêkszo¶ci± konwencji spotykanych
w innych systemach. Po drugie, chcieli¶my sprawdziæ, czy potrafimy to
zrobiæ w³asnymi si³ami :)

Zupe³nie inn± kwesti± jest to, i¿ niektóre rozwi±zania Argante mog± staæ
siê potencjalnie ciekawym k±skiem - jak na przyk³ad mo¿liwo¶ci zarz±dzania
(plug and play) i tworzenia warstwy komunikacyjnej wewn±trz clustrów,
niezale¿nie od stopnia rozproszenia systemów, w sposób transparentny dla
programisty.

Mimo to, nie chcemy, by ten system sta³ siê produktem, dlatego zdecydowali¶my
siê na rozpowszechnianie go od chwili skoñczenia (co, mam nadziejê, nast±pi
w przeci±gu kilku tygodni) na licencji GPL. Produktem mo¿e byæ oprogramwanie,
support, konkretne rozwi±zania.

2) Gdzie widzicie zastosowania Argante?

Wszelkie rozproszone serwery, na których bezpieczeñstwo i efektywno¶æ
jest zagadnieniem krytycznym, wspomniane wy¿ej clustry, oraz wiele innych
zastosowañ. Nie, nie oczekujemy, i¿ Argante stanie siê produktem typu
desk-end - nie chcemy konkurowaæ ani z Microsoftem, ani nie zamierzamy
powtarzaæ sukcesu Linuxa.

3) Czy Argante bêdzie oddzielnym systemem?

Wspomina³em o tym, ale wszystko zale¿y od tego, jak bêdzie siê rozwija³.
System osadzony posiada swoje zalety - m.in. mo¿liwo¶æ dok³adnej
integracji (w ramach wspomnianych rozwi±zañ hybrydowych) z rzeczywistym
systemem, oraz brak konieczno¶ci przenoszenia ca³o¶ci oprogramowania za
jednym zamachem na nowy OS.

4) Jak z przeno¶no¶ci± aplikacji z Unixa?

Nie bêdzie czego¶ takiego, bo Argante operuje na zupe³nie innych zasadach.
Mo¿na mówiæ o przeno¶no¶ci typu DOS <-> Unix - i tu i tu zadzia³a program
"Hello world", ale ¿adne powa¿niejsze zastosowania, ze wzglêdu na ogromne
ró¿nice, nie bêd± przeno¶ne. Dlatego nie starali¶my siê nawet kopiowaæ
jêzyka typu C, choæ oczywi¶cie nic nie stoi na przeszkodzie, by kto¶
napisa³ C dla Argante - aczkowiek nie jest to bezpieczny jêzyk...

5) Czemu Argante ma w³asny jêzyk?

W³a¶ciwie Argante mog³aby operowaæ na podzbiorze Ady - z drugiej strony
wiele konwencji C dotycz±cych sterowania kodem wyda³a nam siê nieszkodliwym
zapo¿yczeniem :) Dlatego AHLL jest po³±czeniem dobrych cech obu jêzyków
w ramach niewielkiego i bardzo ³atwego do opanowania (podobnie jak C, choæ
bez tak chorej arytmetyki na wska¼nikach itp) podzbioru.

6) Czy moge zmieniaæ parametry systemu?

Maksymalna ilo¶æ VCPUs, maksymalny rozmiar stosu, a tak¿e wiêkszo¶æ
innych parametrów charakteryzuj±cych ¶rodowisko, mo¿e byæ modyfikowana
w pliku config.h; nale¿y pamiêtaæ, ¿e zmiana pewnych witalnych
parametrów (np. ilo¶ci rejestrów) mo¿e prowadziæ do niekompatybilno¶ci
/ nieprzeno¶no¶ci programów lub nieprawid³owego ich dzia³ania.

7) Jak Argante wykorzystuje moc procesora?

Gdy wszystkie procesy s± "martwe", oczekuj± na okre¶lony moment lub
znajduj± siê w stanie IOWAIT, zegar VS zwalnia znacznie, oddaj±c wiêkszo¶æ
mocy procesora rzeczywistemu systemowi. W przypadku, gdy choæ jeden lub
wiêcej procesów znajduje siê w stanie WORKING, jest miêdzy nie dzielona
ca³a moc procesora dostêpna dla systemu Argante. Jej warto¶æ mo¿na
kontrolowaæ ustawiaj±c warto¶ci nice oraz schemat schedulingu w
rzeczywistym systemie. 

Mo¿liwe jest uruchomienie dwóch lub wiêcej instancji Argante w
jednym systemie, nale¿y jednak wtedy zwróciæ uwagê na wydajno¶æ,
ewentualnie modyfikuj±æ ustawienia multitaskingu w systemie
rzeczywistym i priorytet procesów Argante.

Je¶li tworzony jest system hybrydowy, gdzie Argante wspó³pracuje
z elementami systemu rzeczywistego, sugerowane jest odpowiednie
ustawienie priorytetów odpowiednio dla Argante i pozosta³ych procesów
systemu rzeczywistego w celu odpowiedniego podzialu czasu procesora.

Wykonywanie prawid³owo skonstruowanych aplikacji Argante nie powinno
powodowaæ zauwa¿alnego obci±¿enia systemu.

9) Przeno¶no¶æ

W obecnej chwili, binarne obrazy wykonywalne nie daj± siê przenosiæ
miêdzy platformami o ró¿nych endianach. Docelowo, w loaderze znajdzie
sie automatyczny translator, obecnie jednak przeno¶ny jest tylko kod
¼ród³owy oraz binarki w obrêbie danego endiana.

¬ród³a powinny byæ przeno¶ne bez ograniczeñ.

10) Kompilator siê wyk³ada?

W przypadku gdyby w czasie kompilacji wystêpowa³y problemy typu "memory
exhausted" lub "segmentation fault", nale¿y wykomentowaæ wszystko w 
linijce CFLAGS= po warto¶ci -Wall w pliku Makefile dla danego systemu
(w katalogu sysdep/). Mo¿e to spowodowaæ nieznaczny spadek wydajno¶ci w 
zamian za przyspieszenie czasu kompilacji i zmniejszenie ilo¶ci zasobów 
potrzebnych przy kompilacji.

11) Gdzie zadzia³a Argante?

Linux		- platforma natywna (wspierane readline)
FreeBSD		- przetestowane
NetBSD          - nie przetestowane, powinno dzia³aæ
OpenBSD		- przetestowane
Solaris		- przetestowane
AIX             - ??? <je¶li masz dostêp, daj znaæ>
IRIX            - przetestowane

...inne systemy?


12) Jakie s± szanse na korzystanie z readline?

Ze wzglêdu na niedoskona³o¶ci biblioteki readline, a z drugiej strony nasz±
(przynajmniej tymczasow±) niechêæ do zajmowania siê kwestiami drugorzêdnymi,
gdy pozostaje do napisania jeszcze wiele istotnego kodu, biblioteka readline
mo¿e byæ wykorzystywana tylko gdy:

- posiadasz Linuxa
- posiadasz nowe libc6 (glibc 2.1.x).

W innej sytuacji wsparcie dla readline nie zostanie wkompilowane. 

13) Chcia³bym co¶ napisaæ - gdzie jest CVS?

Na razie nie ma CVSa i nic nie wskazuje na to, by mia³ powstaæ do czasu
wypuszczenia stabilnej wersji systemu. Na razie CVSem jestem ja - 
lcamtuf@ids.pl - i na ten adres powinni¶cie nadsy³aæ pomys³y lub propozycje,
a tak¿e nadsy³aæ unified diffs (diff -urN) jesli nanosiliscie jakies
poprawki do kodu. Nie nadsy³ajcie w³asnych snapshotów ani diffów uzyskanych
z innymi parametrami - ich rêczne nanoszenie jest do¶æ uci±¿liwe.


----------------
99. DO ZROBIENIA
----------------

- IPC/rIPC - to stuka bulba

- math, advmem - z33d zaczyna

- network - kto chce?

- tlumaczenie dokumentacji - robi Artur...

- optymalizacja interpretera bytecode: proponowa³bym zrobiæ switch,
  który ma case obejmuj±ce ca³y dolny dword instrukcji maszynowej
  (wyd³u¿y kod, ale go bardzo przyspieszy)... - maxiu sie zglosil

- skoñczenie implementacji hll - to ju¿ ja, podla praca :)

- do nastêpnej wersji: packet.so - niskopoziomowy dostêp do socketów

- inne sprytne narzêdzia ;>

-- Micha³ Zalewski
