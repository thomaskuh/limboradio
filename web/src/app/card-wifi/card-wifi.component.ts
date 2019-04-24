import { Component} from '@angular/core';
import {ClientService} from '../client/client.service';
import {MatSnackBar} from '@angular/material';
import {SnackbarMessageComponent} from '../snackbar-message/snackbar-message.component';
import {RadioRequest} from '../client/radio-request';

@Component({
  selector: 'app-card-wifi',
  templateUrl: './card-wifi.component.html',
  styleUrls: ['./card-wifi.component.scss']
})
export class CardWifiComponent {

  item = new RadioRequest();
  disabled = false;

  constructor(private client: ClientService, private snackBar: MatSnackBar) {
    this.item.wifiName = '';
    this.item.wifiPass = '';
  }

  save() {
    if(this.disabled) return;
    this.disabled = true;

    this.client.saveThatThang(this.item).subscribe(
      result => {
        this.snackBar.openFromComponent(SnackbarMessageComponent, {duration: 3000, data: 'DONE'});
        this.disabled = false;
      },
      error => {
        this.disabled = false;
        }
    );
  }

}
