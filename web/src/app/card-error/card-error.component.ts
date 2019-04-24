import {Component, Input} from '@angular/core';

@Component({
  selector: 'app-card-error',
  templateUrl: './card-error.component.html',
  styleUrls: ['./card-error.component.scss']
})
export class CardErrorComponent {

  @Input()
  error: string = null;

  constructor() { }

}
