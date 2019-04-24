import {Component, Input} from '@angular/core';
import {RadioResponse} from '../client/radio-response';
import {MatSliderChange} from '@angular/material';
import {ClientService} from '../client/client.service';
import {RadioRequest} from '../client/radio-request';

@Component({
  selector: 'app-card-status',
  templateUrl: './card-status.component.html',
  styleUrls: ['./card-status.component.scss']
})
export class CardStatusComponent {

  @Input()
  state: RadioResponse = new RadioResponse();

  constructor(private client: ClientService) { }

  setVolume(e: MatSliderChange) {
    const req = new RadioRequest();
    req.volume =  this.state.volume;
    this.client.saveThatThang(req).subscribe();
  }

}
