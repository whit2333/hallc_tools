#/bin/sh

function print_the_help {
  echo "USAGE: get_setting "
  echo "  OPTIONS: "
  echo "            -p,--p-hms         HMS momentum"
  echo "            -P,--p-shms        SHMS momentum"
  echo "            -t,--theta-hms     HMS theta"
  echo "            -T,--theta-shms    SHMS theta"
  exit 
}

function yes_or_no {
  while true; do
    read -p "$* [y/n]: " yn
    case $yn in
      [Yy]*) return 0 ;;
      [Nn]*) echo "No entered" ; return 1 ;;
    esac
  done
}


if [[ $# -eq 0 ]] ; then
  print_the_help
  exit 
fi


POSITIONAL=()
while [[ $# -gt 0 ]]
do
  key="$1"

  case $key in
    -h|--help)
      shift # past argument
      print_the_help
      ;;
    -p|--p-hms)
      hms_p="$2"
      shift # past argument
      shift # past value
      ;;
    -P|--p-shms)
      shms_p="$2"
      shift # past argument
      shift # past value
      ;;
    -t|--theta-hms)
      hms_th="$2"
      shift # past argument
      shift # past value
      ;;
    -T|--theta-shms)
      shms_th="$2"
      shift # past argument
      shift # past value
      ;;
    -b|--target)
      targ="$2"
      shift # past argument
      shift # past value
      ;;
    *)    # unknown option
      #POSITIONAL+=("$1") # save it in an array for later
      echo "unknown option $1"
      print_the_help
      shift # past argument
      ;;
  esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

# target
if [[ -z "$targ"  ]] ; 
then 
  print_the_help
  echo "Error target argument, --target, required."
  exit
fi

# HMS
if [[ -z "$hms_p"  ]] ; 
then 
  print_the_help
  echo "Error HMS momentum argument, --p-hms, required."
  exit
fi
if [[ -z "$hms_th"  ]] ; 
then 
  print_the_help
  echo "Error HMS angle argument, --theta-hms, required."
  exit
fi

# SHMS
if [[ -z "$shms_p"  ]] ; 
then 
  print_the_help
  echo "Error SHMS momentum argument, --p-shms, required."
  exit
fi
if [[ -z "$shms_th"  ]] ; 
then 
  print_the_help
  echo "Error SHMS angle argument, --theta-shms, required."
  exit
fi


#hms_p=$(caget hcHMSMomentum  | sed 's/hcHMSMomentum//')
#shms_p=$(caget hcSHMSMomentum  | sed 's/hcSHMSMomentum//')
#hms_th=$(caget hcHMSCorrectedAngle  | sed 's/hcHMSCorrectedAngle//')
#shms_th=$(caget hcSHMSCorrectedAngle  | sed 's/hcSHMSCorrectedAngle//')
#targ=$(caget hcBDSSELECT  | sed 's/hcBDSSELECT//')
root -l -b -q "/group/c-csv/cdaq/csv_run_plan/run_plan/src/master_settings.cxx(${hms_p},${hms_th},${shms_p},${shms_th},${targ})" 2> /dev/null | tail -n 1  | sed 's/ / /'
