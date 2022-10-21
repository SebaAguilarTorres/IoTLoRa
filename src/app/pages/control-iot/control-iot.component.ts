import { Component, OnInit } from '@angular/core';
import { AngularFireDatabase } from '@angular/fire/compat/database';

@Component({
  selector: 'app-control-iot',
  templateUrl: './control-iot.component.html',
  styleUrls: ['./control-iot.component.scss'],
})
export class ControlIotComponent implements OnInit {

  seccion: 'temp' | 'contr' | 'hist' = 'temp';
  mediciones: Mediciones[] = []; 

  lastMedicion: Mediciones = {
    temperatura: null,
    time: null,
  }

  umbral: number = 20;
  ventilador_state: boolean = null;
  ventilador: number = 0;

  manual: boolean = false;


  constructor(public database: AngularFireDatabase) {
        this.leerMediciones();
        this.leerStateVentilador();
        this.setManual();
   }

  ngOnInit() {}

  leerMediciones() {

      const path = 'mediciones/';
      
      this.database.list<Mediciones>(path).valueChanges().subscribe( res => {
          console.log('mediciones -> ', res);
          this.mediciones = res;
          this.mediciones.reverse();
          this.lastMedicion = this.mediciones[0];
      })

  }

  leerStateVentilador(){
    const path = 'ventilador_state';
    this.database.object<boolean>(path).valueChanges().subscribe(res => {
      if (res !== undefined) {
        this.ventilador_state = res;
      }
    })
  }

  segmentChanged(ev: any) {
      this.seccion = ev.detail.value;
    // console.log('ev.detail.value ->', ev.detail.value);
  }


  rangeChange(ev: any) {
    this.umbral = ev.detail.value;
    //console.log('ev.detail.value ->', ev.detail.value);
    const path = 'umbral';
    this.database.object(path).set(this.umbral);

  }

  toogleChange(ev: any) {
    console.log('ev ->', ev.detail.checked);
    this.manual = ev.detail.checked;
    if (!this.manual) {
      const path = 'ventilador';
      this.database.object(path).set(0);
    }
  }

  toogleChangePrender(ev: any) {
    const path = 'ventilador';
    if(ev.detail.checked) {
      this.database.object(path).set(2);
    } else {
      this.database.object(path).set(1);
    }
    
  }

  setManual(){
    const path = 'ventilador';
    this.database.object<number>(path).valueChanges().subscribe(res => {
          console.log('setManual() ->', res);
          if (res > 0) {
            this.manual = true;
          } else {
            this.manual = false;
          }
    });
  }

  }


  interface Mediciones {
    temperatura: number;
    time: number;
  }




