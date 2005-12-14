;;
;;  spanish.nsh
;;
;;  Galician language strings for the Windows Gaim NSIS installer.
;;  Translator: Ignacio Casal Quinteiro 
;;
;; 
;;

; Startup GTK+ check
!define GTK_INSTALLER_NEEDED			"O entorno de execuci�n de GTK+ falta ou necesita ser actualizado.$\rPor favor, instale a versi�n v${GTK_VERSION} do executable GTK+ ou algunha posterior."

; License Page
!define GAIM_LICENSE_BUTTON			"Seguinte >"
!define GAIM_LICENSE_BOTTOM_TEXT		"$(^Name) distrib�ese baixo a licencia GPL. Esta licencia incl�ese aqu� s� con prop�sito informativo: $_CLICK"

; Components Page
!define GAIM_SECTION_TITLE			"Cliente de mensaxer�a instant�nea de Gaim (necesario)"
!define GTK_SECTION_TITLE			"Entorno de execuci�n de GTK+ (necesario)"
!define GTK_THEMES_SECTION_TITLE		"Temas GTK+" 
!define GTK_NOTHEME_SECTION_TITLE		"Sen tema"
!define GTK_WIMP_SECTION_TITLE		"Tema Wimp"
!define GTK_BLUECURVE_SECTION_TITLE		"Tema Bluecurve"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE	"Tema Light House Blue"
!define GAIM_SECTION_DESCRIPTION		"Ficheiros e dlls principais de Core"
!define GTK_SECTION_DESCRIPTION		"Unha suite de ferramentas GUI multiplataforma, utilizada por Gaim"
!define GTK_THEMES_SECTION_DESCRIPTION	"Os temas poden cambiar a apariencia de aplicaci�ns GTK+."
!define GTK_NO_THEME_DESC			"Non instalar un tema GTK+"
!define GTK_WIMP_THEME_DESC			"GTK-Wimp (Windows impersonator) � un tema GTK que se mestura moi bien co entorno de escritorio de Windows."
!define GTK_BLUECURVE_THEME_DESC		"O tema Bluecurve."
!define GTK_LIGHTHOUSEBLUE_THEME_DESC	"O tema Lighthouseblue."

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"Atopouse unha versi�n antiga do executable de GTK+. �Desexa actualizala?$\rObservaci�n: Gaim non funcionar� a menos que o faga."

; Installer Finish Page
!define GAIM_FINISH_VISIT_WEB_SITE		"Visite a p�xina Web de Gaim Windows"

; Gaim Section Prompts and Texts
!define GAIM_UNINSTALL_DESC			"Gaim (s�lo eliminar)"
!define GAIM_PROMPT_WIPEOUT			"O seu directorio antigo de Gaim vai ser borrado. �Desexa continuar?$\r$\rObservaci�n: calquer aplique non est�ndar que puidera haber instalado borrarase.$\rIsto non afectar� �s s�as preferencias de usuario en Gaim."
!define GAIM_PROMPT_DIR_EXISTS		"O directorio de instalaci�n que especificou xa existe. Todos os contidos$\rser�n borrados. �Desexa continuar?"

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"Erro ao instalar o executable GTK+."
!define GTK_BAD_INSTALL_PATH			"Non se puido acceder ou crear a ruta que vd. indicou."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"Non ten permisos para instalar un tema GTK+."

; Uninstall Section Prompts
!define un.GAIM_UNINSTALL_ERROR_1         "O desinstalador non puido atopar as entradas no rexistro de Gaim.$\r� probable que outro usuario instalara a aplicaci�n."
!define un.GAIM_UNINSTALL_ERROR_2         "Non ten permisos para desinstalar esta aplicaci�n."
